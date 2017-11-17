////////////////////////////////////////////////////////////////////////////////
//
//  @file: bnb.cpp
//  @author: Tiago Lobato Gimenes (tlgimenes@gmail.com)
//  @time: 2017-11-13T17:49:34.594Z
//
//  @brief This is the implementation of the scene ordering problem.
//
//  TODO
//
////////////////////////////////////////////////////////////////////////////////

#include "common.hpp"
#include "simmulated_anneling.hpp"

////////////////////////////////////////////////////////////////////////////////
// BnB functions
int lower_bound(solution& sol);
void explore();

////////////////////////////////////////////////////////////////////////////////
// Main function. Reads input and call other methods
int main(int argc, char **argv)
{
  // Sets output format to branch and bound
  is_bnb = true;

  // Signal handling
  signal(SIGINT, print_and_exit);

  // Read from input file
  read_input(argv[1]);

  // Lets do our heuristics first to find a good bound for the algorithm
  simmulated_anneling(3000);
  
  // Creates tree root with empty solution
  sol_tree.push_back(solution(nscenes));

  // Explores solution tree and updates best solution so far
  explore();

  // Exploration is finished, prints and exit
  print_and_exit();

  return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

int k1k2(solution& sol, std::vector<int>& bl, std::vector<int>& br)
{
  int lmost, rmost, rlmost, lrmost, partial;
  int cost = 0;

  for(int i=0; i < nactors; i++)
  {
    lmost = rlmost = lrmost = rmost = -1;
    partial = 0;

    for(int j=0; j < sol.lactive; j++) {
      if(t[i][sol.sol[j]] == true) {
        if(lmost == -1) lmost = j;

        lrmost = j;
      }
    }

    for(int j=sol.sol.size()-1; j >= sol.ractive; j--) {
      if(t[i][sol.sol[j]] == true) {
        if(rmost == -1) rmost = j;

        rlmost = j;
      }
    }

    if(lmost != -1 && rmost != -1) {
      partial = (rmost - lmost + 1 - wdays[i]);
    }
    else if(lmost != -1) {
      for(int j=lmost+1; j < lrmost; j++) partial += (1 - (int)t[i][sol.sol[j]]);

      if(partial > 0) bl.push_back(i);
    }
    else if(rmost != -1) {
      for(int j=rmost-1; j > rlmost; j--) partial += (1 - (int)t[i][sol.sol[j]]);

      if(partial > 0) br.push_back(i);
    }

    cost += partial * costs[i];
  }

  return cost;
}

void compute_Q(solution& sol, std::vector<int>& bl, std::vector<std::pair<int, int>>& Q)
{
  // List of tuples for (scene id, actors in the scene, scene cost)
  std::vector<std::tuple<int, std::vector<int>, int>> candidates;

  // Reserves right size for candidates
  candidates.reserve(sol.comp.size());

  // Computes number of actors in bl per scene
  for(int scene: sol.comp) {
    candidates.push_back(std::make_tuple(scene, std::vector<int>(), 0));

    for(int actor: bl) {
      if(t[actor][scene]) {
        std::get<1>(candidates.back()).push_back(actor);
        std::get<2>(candidates.back()) += costs[actor];
      }
    }
  }

  // Sorts scenes in increasing order of number of actors per scene
  using elem_t = std::tuple<int, std::vector<int>, int>;
  std::sort(candidates.begin(), candidates.end(), [](const elem_t& i, const elem_t& j) { return std::get<1>(i).size() < std::get<1>(j).size(); });

  // Now lets set the Q set.

  // It serves for not adding two scenes with the same actor to Q
  std::vector<bool> used_actors(nactors, false);
  // iterates over the scenes candidates
  for(auto& candidate: candidates)
  {
    int scene = std::get<0>(candidate); // scene id
    bool add_scene = true; // always try to add scene
    std::vector<int>& actors = std::get<1>(candidate); // actors in that scene
    int cost = std::get<2>(candidate);

    // Checks if an actor was used in this scene before
    for(int actor: actors) { if(used_actors[actor]) { add_scene = false; break; } }

    // If none of the actors of this scene were used yet, try adding the scene
    if(add_scene) {
      Q.push_back(std::make_pair(scene, cost)); // adds the scene
      for(int actor: actors) used_actors[actor] = true; // adds to used actors
    }
  }

  using q_t = std::pair<int, int>;
  std::sort(Q.begin(), Q.end(), [](const q_t& i, const q_t& j) { return i.second > j.second; });
}

int k3(solution& sol, std::vector<int>& bl)
{
  std::vector<std::pair<int, int>> Q;
  int cost = 0;

  // Computes Q according to the description of the problem in decreasing weight
  compute_Q(sol, bl, Q);

  for(int i=0; i < (int)Q.size(); i++)
  {
    cost += i * Q[i].second;
  }

  return cost;
}

int k4(solution& sol, std::vector<int>& br)
{
  std::vector<std::pair<int, int>> Q;
  int cost = 0;

  // Computes Q according to the description of the problem in decreasing weight
  compute_Q(sol, br, Q);

  for(int i=0; i < (int)Q.size(); i++)
  {
    cost += i * Q[i].second;
  }

  return cost;
}

////////////////////////////////////////////////////////////////////////////////
// Computes the lower bound by using the function described on the pdf by
// computing the acummulated sum of k1,k2,k3 and k4.
int lower_bound(solution& sol)
{
  std::vector<int> bl, br;

  int bound = k1k2(sol, bl, br);
  bound += k3(sol, bl);
  bound += k4(sol, br);

  return bound;
}

////////////////////////////////////////////////////////////////////////////////
// Explores the solution tree by using branch and bound - best fit
void explore()
{
  int mem = 1; // used at memory management

  while(sol_tree.size() > 0 && sol_tree.front().lower_bound < best_sol.lower_bound)
  {
    // Pops head from the priority queue
    solution node = sol_tree.front();
    std::pop_heap(sol_tree.begin(), sol_tree.end());
    sol_tree.pop_back();

    // Number of explored nodes
    nexplored++;

    // If we've found a possible solution
    if(node.comp.size() == 0)
    {
      // If the solution is actually better
      if(node.lower_bound < best_sol.lower_bound) {
        // the solution is actually better
        update_solution(node);
      }
    } else
    {
      int st_size = sol_tree.size(), idx = -1, min = -1;

      if(node.lactive == (int)node.sol.size() - node.ractive) {
        idx = ++node.lactive-1;
      } else {
        idx = --node.ractive;
        min = (node.ractive == node.sol.size()-1) ? node.sol[node.lactive-1] : -1;
      }

      // for each possible scene, creates a new solution with it and inserts it
      // into the solution tree if its lower bound allows
      for(int it=0; it < node.comp.size(); it++) {
        int scene = node.comp[it];

        if(min < scene) { // breaks simetry
          solution new_node(node);

          new_node.comp.erase(new_node.comp.begin()+it, new_node.comp.begin()+it+1);
          new_node.sol[idx] = scene;
          new_node.lower_bound = lower_bound(new_node);

          solution greedy(new_node);

          // Asks for a greedy solution
          greedy_solution(greedy);

          // mature node condition
          if(new_node.lower_bound < greedy.lower_bound && new_node.lower_bound < best_sol.lower_bound) sol_tree.push_back(new_node);
          // If greedy is better than current, update best solution so far
          if(greedy.lower_bound < best_sol.lower_bound) update_solution(greedy);
        }
      }

      // updates heap with freshly added nodes
      for(int it = std::max(st_size, 1); it <= (int)sol_tree.size(); it++) {
        std::push_heap(sol_tree.begin(), sol_tree.begin()+it);
      }
    }

    // Try not to take too much memory by removing some inactive nodes from
    // time to time
    if(nexplored/mem > 1e5 && sol_tree.size() > 5e5)
    {
      mem++;

      for(int i=sol_tree.size()-1; i >= 0; i--) {
        if(sol_tree[i].lower_bound >= best_sol.lower_bound)
          sol_tree.erase(sol_tree.begin()+i, sol_tree.begin()+i+1);
      }

      std::make_heap(sol_tree.begin(), sol_tree.end());

      // std::cerr << "After removal " << sol_tree.size() << std::endl;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
