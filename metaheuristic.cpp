////////////////////////////////////////////////////////////////////////////////
//
//  @file: simmulated_anneling.hpp
//  @time: 2017-11-17T05:18:14.072Z
//
//  @brief TODO
//
////////////////////////////////////////////////////////////////////////////////

#include "metaheuristic.hpp"

#include <chrono>

////////////////////////////////////////////////////////////////////////////////
// Constants
const float T_ZERO = 100; // Initial temperature
const float K = 1; // Constant for probability
const int N_STEPS = 10; // Number maximum of steps without improving solution
const int N_MEMBERS = 25; // Size of population (use odd numbers to crossover all but the fittest)
const float MUTATION_RATE = 0.05f; // Probability of mutating a gene
const float CROSSOVER_RATE = 0.5f; // Min percentage of genes to be preserved on crossover

////////////////////////////////////////////////////////////////////////////////

int get_cost(solution& sol)
{
  int cost = 0;
  // Calculates cost for each actor
  for (int i = 0; i < nactors; i++) {
    int first_day = 0, last_day = nscenes - 1;
    // Finds first day of work
    for (int j = 0; j < nscenes; j++) {
      if (t[i][sol.sol[j]]) {
        first_day = j;
        break;
      }
    }
    // Finds last day of work
    for (int j = nscenes-1; j >= 0; j--) {
      if (t[i][sol.sol[j]]) {
        last_day = j;
        break;
      }
    }
    // Sums actor cost to total
    cost += (last_day - first_day + 1 - wdays[i]) * costs[i];
  }
  return cost;
}

////////////////////////////////////////////////////////////////////////////////

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
  }
  sol.comp.clear();

  // Update lower bound
  sol.lower_bound = get_cost(sol);
}

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

float get_probability(float cost_diff, float temperature)
{
  return std::exp(-cost_diff / (K * temperature));
}

////////////////////////////////////////////////////////////////////////////////

float update_temperature(float fraction)
{
  fraction = std::min(fraction, 1.0f);
  return (1 - fraction) * T_ZERO;
}

////////////////////////////////////////////////////////////////////////////////

void simmulated_anneling(float time_max)
{
  // Runs greedy algorithm for initial solution
  best_sol = solution(nscenes);
  greedy_solution(best_sol);
  solution cur_sol(best_sol);
  std::cout << "Iteration 0 -> Best: " << best_sol.lower_bound << std::endl;

  // Initialization
  auto time_start = std::chrono::high_resolution_clock::now();
  auto time_now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float, std::milli> time_delta = time_start - time_now;
  float temperature = T_ZERO;

  // Run until temperature reaches 0
  int iteration = 0;
  while (temperature > 0) {
    // Generate new solution for a maximum of N steps without improvement
    for (int i = 0; i < N_STEPS; i++) {
      // Gets new solution
      iteration++;
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

          // Prints current results
          std::cout << "Iteration " << iteration;
          std::cout << " -> Best: " << best_sol.lower_bound;
          std::cout << " / Temperature: " << temperature;
          std::cout << " / Time: " << time_delta.count() / 1000 << std::endl;
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

  // Prints final results
  std::cout << "Iteration " << iteration;
  std::cout << " -> Best: " << best_sol.lower_bound;
  std::cout << " / Temperature: " << temperature;
  std::cout << " / Time: " << time_delta.count() / 1000 << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void random_solution(solution& sol)
{
  // List of scenes
  std::vector<int> scenes;
  scenes.reserve(sol.comp.size());

  // Gets only remaining scenes
  for (int scene: sol.comp) {
    scenes.push_back(scene);
  }

  // Random shuffle
  std::random_shuffle(scenes.begin(), scenes.end());

  // Completes solution
  for (auto& scene: scenes) {
    int idx = ++sol.lactive - 1;
    sol.sol[idx] = scene;
  }
  sol.comp.clear();

  // Update lower bound
  sol.lower_bound = get_cost(sol);
}

////////////////////////////////////////////////////////////////////////////////

solution get_fittest(std::vector<solution>& population, int& total_fitness)
{
  int fittest_idx = 0;
  total_fitness = 0;
  // Search fittest individual of population
  for (int i = 1; i < population.size(); i++) {
    if (population[i].lower_bound <= population[fittest_idx].lower_bound) {
      fittest_idx = i;
    }
    total_fitness += population[i].lower_bound;
  }
  // Checks if fittest individual is also best solution
  solution fittest = population[fittest_idx];
  if (fittest.lower_bound < best_sol.lower_bound) {
    best_sol = fittest;
  }
  return fittest;
}

////////////////////////////////////////////////////////////////////////////////

void crossover(solution& individual_1, solution& individual_2)
{
  // Copies individual 1
  solution temp_individual_1 = individual_1;

  // Crossover range
  int min_range = (int)std::ceil(CROSSOVER_RATE * nscenes);
  int min = rand() % (nscenes - min_range);
  int max = min + rand() % (nscenes - min);
  int idx1, idx2;

  // Keep individual 1 genes on crossover range
  // and copies remaining genes from individual 2 on order
  auto it_min = individual_1.sol.begin() + min;
  auto it_max = individual_1.sol.begin() + max + 1;
  idx2 = 0;
  for (idx1 = 0; idx1 < nscenes - 1; idx1++) {
    // Finds next gene not already present on new individual
    while (std::find(it_min, it_max, individual_2.sol[idx2]) == it_max) {
      idx2++;
    }
    // Replaces genes outside crossover range
    if (idx1 < min && idx1 > max) {
      individual_1.sol[idx1] = individual_2.sol[idx2];
    }
  }

  // Keep individual 2 genes on crossover range
  // and copies remaining genes from individual 1 on order
  it_min = individual_2.sol.begin() + min;
  it_max = individual_2.sol.begin() + max + 1;
  idx1 = 0;
  for (idx2 = 0; idx2 < nscenes - 1; idx2++) {
    // Finds next gene not already present on new individual
    while (std::find(it_min, it_max, temp_individual_1.sol[idx1]) == it_max) {
      idx1++;
    }
    // Replaces genes outside crossover range
    if (idx2 < min && idx2 > max) {
      individual_2.sol[idx2] = temp_individual_1.sol[idx1];
    }
  }

  // Updates fitness
  individual_1.lower_bound = get_cost(individual_1);
  individual_2.lower_bound = get_cost(individual_2);
}

////////////////////////////////////////////////////////////////////////////////

void mutate(solution& individual)
{
  // Tries mutating every scene by swapping
  for (int idx1 = 0; idx1 < nscenes - 1; idx1++) {
    float mutation_chance = (double)rand() / (RAND_MAX);
    if (mutation_chance < MUTATION_RATE) {
      // Gets random scene after this one
      int idx2 = idx1 + 1 + rand() % (nscenes - idx1 - 1);
      // Swaps scenes
      int scene1 = individual.sol[idx1];
      individual.sol[idx1] = individual.sol[idx2];
      individual.sol[idx2] = scene1;
    }
  }
  // Updates fitness
  individual.lower_bound = get_cost(individual);
}

////////////////////////////////////////////////////////////////////////////////

int roulette(std::vector<solution>& population, int total_fitness)
{
  // Randomizes roulette range
  float random_probability = (double)rand() / (RAND_MAX);
  // Searches for individual on this range
  float current_probability = 0;
  for (int i = 0; i < N_MEMBERS; i++) {
    float fitness_ratio = population[i].lower_bound / (float)total_fitness;
    current_probability += (1 - fitness_ratio) / (N_MEMBERS - 1);
    if (current_probability >= random_probability) {
      return i;
    }
  }
  // Should not reach this point
  return rand() % N_MEMBERS;
}

////////////////////////////////////////////////////////////////////////////////

void evolve_population(std::vector<solution>& population, int total_fitness)
{
  // Creates new population
  std::vector<solution> new_population;
  new_population.reserve(N_MEMBERS);

  // Saves fittest individual
  solution fittest = get_fittest(population, total_fitness);
  new_population.push_back(fittest);

  // Generates new individuals
  for (int i = 1; i < N_MEMBERS; i = i + 2) {
    // Gets parents and creates children
    int parent_idx1 = roulette(population, total_fitness);
    int parent_idx2 = roulette(population, total_fitness);

    solution child_1 = population[parent_idx1];
    solution child_2 = population[parent_idx2];

    // Crossovers parents genes
    crossover(child_1, child_2);

    // Mutates children
    mutate(child_1);
    mutate(child_2);

    // Saves children to new population
    new_population.push_back(child_1);
    new_population.push_back(child_2);
  }

  // Updates population
  population = new_population;
}

////////////////////////////////////////////////////////////////////////////////

void genetic_algorithm(float time_max)
{
  // Creates population
  std::vector<solution> population;
  population.reserve(N_MEMBERS);

  // Runs greedy algorithm for initial best solution
  best_sol = solution(nscenes);
  greedy_solution(best_sol);
  population.push_back(best_sol);

  // Randomizes individuals
  for (int i = 1; i < N_MEMBERS; i++) {
    solution new_sol(nscenes);
    random_solution(new_sol);
    population.push_back(new_sol);
  }

  // Updates best solution
  int total_fitness = 0;
  solution fittest = get_fittest(population, total_fitness);
  std::cout << "Generation 0 -> Fittest: " << fittest.lower_bound << std::endl;

  // Timer initialization
  auto time_start = std::chrono::high_resolution_clock::now();
  auto time_now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float, std::milli> time_delta = time_start - time_now;

  // Runs until timeout
  int generation = 0;
  while (time_delta.count() < time_max) {
    // Evolves population
    generation++;
    evolve_population(population, total_fitness);

    // Gets fittest solution and total fitness
    solution new_fittest = get_fittest(population, total_fitness);

    // Updates elapsed time
    time_now = std::chrono::high_resolution_clock::now();
    time_delta = time_now - time_start;

    // Prints generation results
    if (new_fittest.lower_bound < fittest.lower_bound || time_delta.count() >= time_max) {
      fittest = new_fittest;
      std::cout << "Generation " << generation;
      std::cout << " -> Fittest: " << fittest.lower_bound;
      std::cout << " / Time: " << time_delta.count() / 1000 << std::endl;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
