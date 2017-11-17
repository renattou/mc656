#include "common.hpp"
#include <chrono>

////////////////////////////////////////////////////////////////////////////////
// Constants
const float T_ZERO = 100; // Initial temperature
const float TIMEOUT = 30000; // Time in which temperature goes to zero
const float K = 1; // Constant for probability
const int N = 10; // Number maximum of steps without improving solution

////////////////////////////////////////////////////////////////////////////////
// Heuristic functions
void simmulated_anneling(float time_max);

////////////////////////////////////////////////////////////////////////////////
// Main function. Reads input and call other methods
int main(int argc, char **argv)
{
  // Signal handling
  signal(SIGINT, print_and_exit);

  // Reads from input file
  read_input(argv[1]);

  // Runs simmulated anneling until timeout
  simmulated_anneling(TIMEOUT);

  // Prints and exit
  print_and_exit();

  return EXIT_SUCCESS;
}

int get_cost(solution& sol)
{
  int cost = 0;
  for (int i = 0; i < nactors; i++) {
    int first_day = 0, last_day = nscenes - 1;
    for (int j = 0; j < nscenes; j++) {
      if (t[i][sol.sol[j]]) {
        first_day = j;
        break;
      }
    }
    for (int j = nscenes-1; j >= 0; j--) {
      if (t[i][sol.sol[j]]) {
        last_day = j;
        break;
      }
    }
    cost += (last_day - first_day + 1 - wdays[i]) * costs[i];
  }
  return cost;
}

void greedy_solution(solution& sol)
{
  // List of pairs (scene, scene cost)
  using scenes_t = std::pair<int, int>;
  std::vector<scenes_t> scenes;
  scenes.reserve(sol.comp.size());

  // Gets only remaining scenes
  for (int scene: sol.comp) {
    scenes.push_back(std::make_pair(scene, scene_costs[scene]));
  }

  // Sorts scenes in decreasing order of scene cost
  std::sort(scenes.begin(), scenes.end(), [](const scenes_t& i, const scenes_t& j) { return i.second > j.second; });

  // Completes solution
  for (auto& scene: scenes) {
    int idx = ++sol.lactive - 1;
    sol.sol[idx] = scene.first;
    //sol.comp.erase(sol.comp.begin()+idx, sol.comp.begin()+idx+1);
  }

  // Update lower bound
  sol.lower_bound = get_cost(sol);
}

void get_neighbour(solution& sol, solution& new_sol)
{
  // Copies solution
  new_sol = sol;

  // Randomizes two scenes to swap
  int idx1 = rand() % nscenes;
  int idx2 = idx1;
  while (idx1 == idx2) {
    idx2 = rand() % nscenes;
  }

  // Swaps scenes
  new_sol.sol[idx1] = sol.sol[idx2];
  new_sol.sol[idx2] = sol.sol[idx1];

  // Update lower bound
  new_sol.lower_bound = get_cost(new_sol);
}

float get_probability(float cost_diff, float temperature)
{
  return std::exp(-cost_diff / (K * temperature));
}

float update_temperature(float fraction)
{
  return (1 - fraction) * T_ZERO;
}

void simmulated_anneling(float time_max)
{
  // Runs greedy algorithm for initial solution
  best_sol = solution(nscenes);
  greedy_solution(best_sol);
  solution cur_sol(best_sol);

  // Initialization
  auto time_start = std::chrono::high_resolution_clock::now();
  auto time_now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float, std::milli> time_delta = time_start - time_now;
  float temperature = T_ZERO;

  // Run until temperature reaches 0
  while (temperature >= 0) {
    // Generate new solution for a maximum of N steps without improvement
    for (int i = 0; i < N; i++) {
      // Gets new solution
      solution new_sol;
      get_neighbour(cur_sol, new_sol);

      // Gets difference in cost
      int cost_diff = new_sol.lower_bound - cur_sol.lower_bound;

      // Accepts better solution always
      if (cost_diff <= 0) {
        cur_sol = new_sol;
        i = -1; // Resets max steps counter

        // Accepts best solution
        if (new_sol.lower_bound < best_sol.lower_bound) {
          best_sol = new_sol;
        }
      }
      // Accepts worse solution with some probability
      else {
        float probability = get_probability(cost_diff, temperature);
        float random_probability = (double) rand() / (RAND_MAX);

        if (random_probability >= probability) {
          cur_sol = new_sol;
        }
      }

      // Updates and check timer
      time_now = std::chrono::high_resolution_clock::now();
      time_delta = time_now - time_start;
      if (time_delta.count() >= time_max) {
        break;
      }
    }

    // Updates temperature
    temperature = update_temperature(time_delta.count() / time_max);
  }
}