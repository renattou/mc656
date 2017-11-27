////////////////////////////////////////////////////////////////////////////////
//
//  @file: bnb.cpp
//  @author: Tiago Lobato Gimenes (tlgimenes@gmail.com)
//  @time: 2017-11-13T17:49:34.594Z
//
//  @brief This is the implementation of the scene ordering problem.
//
////////////////////////////////////////////////////////////////////////////////

#include "common.hpp"
#include "metaheuristic.hpp"

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
  genetic_algorithm(100);

  // Creates tree root with empty solution
  sol_tree.push_back(solution(nscenes));

  // Explores solution tree and updates best solution so far
  explore();

  // Exploration is finished, prints and exit
  print_and_exit();

  return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// Computes the sum of k1 and k2 bounds as described in the paper
int k1k2(solution& sol, std::vector<short>& bl, std::vector<short>& br)
{
  short lmost, rmost, lpartial, rpartial, partial;
  int cost = 0;

  for(short i=0; i < nactors; i++)
  {
    lmost = rmost = -1;
    lpartial = rpartial = partial = 0;

    // computes index of left most 1 in the scenes order and set it to lmost
    // Also computes the right most 1 in the left set and set it to lrmost
    short j, ac = 0;
    for(j=0; j < sol.lactive; j++) { if(t[i][sol.sol[j]]) { lmost = j; break; } }
    for(; j < sol.lactive; j++) {
      if(t[i][sol.sol[j]]) { lpartial += ac; ac = 0; }
      else ac++;
    }

    // computes index of right most 1 in the scenes order and set it to rmost
    // Also computes the left most 1 in the right set and set it to rlmost
    ac = 0;
    for(j=sol.sol.size()-1; j >= sol.ractive; j--) { if(t[i][sol.sol[j]]) { rmost = j; break; } }
    for(; j >= sol.ractive; j--) {
      if(t[i][sol.sol[j]]) { rpartial += ac; ac = 0; }
      else ac++;
    }

    // If the waiting time is totally defined
    if(lmost != -1 && rmost != -1) {
      partial = (rmost - lmost + 1 - wdays[i]);
    }
    // If only the left set is defined
    else if(lmost != -1 && lpartial > 0) {
      partial = lpartial;
      bl.push_back(i);
    }
    // If only the right set is defined
    else if(rmost != -1 && rpartial > 0) {
      partial = rpartial;
      br.push_back(i);
    }

    // Updates cost for each actor
    cost += partial * costs[i];
  }

  return cost;
}

// Computes Q set as defined in the paper
void compute_Q(solution& sol, std::vector<short>& bl, std::vector<int>& Q)
{
  // List of tuples for (actors in the scene, scene cost)
  using elem_t = std::pair<std::vector<short>, int>;
  std::vector<elem_t> candidates;

  // Computes number of actors in bl per scene
  for(auto scene: sol.comp) {
    std::vector<short> actors;
    int cost = 0;

    for(short actor: bl) {
      if(t[actor][scene]) {
        actors.push_back(actor);
        cost += costs[actor];
      }
    }

    if(cost > 0) // only add candidates with positive costs
      candidates.push_back(std::make_pair(actors, cost));
  }

  // Sorts scenes in increasing order of number of actors per scene. Use costs as tie breaker
  std::sort(candidates.begin(), candidates.end(), [](const elem_t& i, const elem_t& j)
  {
    int si = i.first.size(), sj = j.first.size();
    return (si == sj) ? i.second < j.second : si < sj;
  });

  // Now lets set the Q set.

  // It serves for not adding two scenes with the same actor to Q
  std::vector<bool> used_actors(nactors, false);
  // iterates over the scenes candidates
  for(auto& candidate: candidates)
  {
    bool add_scene = true; // always try to add scene

    // Checks if an actor was used in this scene before, and add it to used actors
    for(short actor: candidate.first) {
      if(used_actors[actor]) add_scene = false;
      used_actors[actor] = true;
    }

    // If none of the actors of this scene were used yet, try adding the scene
    if(add_scene) Q.push_back(candidate.second); // adds the scene cost
  }

  std::sort(Q.begin(), Q.end(), [](int a, int b) { return a >= b; });
}

// Computes k3 as defined in the paper
int k3(solution& sol, std::vector<short>& bl)
{
  std::vector<int> Q;
  int cost = 0;

  // Computes Q according to the description of the problem in decreasing weight
  compute_Q(sol, bl, Q);

  // Computes the cost
  for(int i=0; i < (int)Q.size(); i++) cost += i * Q[i];

  return cost;
}

// Computes k4 as defined in the paper
int k4(solution& sol, std::vector<short>& br)
{
  std::vector<int> Q;
  int cost = 0;

  // Computes Q according to the description of the problem in decreasing weight
  compute_Q(sol, br, Q);

  // Computes the cost
  for(int i=0; i < (int)Q.size(); i++) cost += i * Q[i];

  return cost;
}

////////////////////////////////////////////////////////////////////////////////
// Computes the lower bound by using the function described on the pdf by
// computing the acummulated sum of k1,k2,k3 and k4.
int lower_bound(solution& sol)
{
  std::vector<short> bl, br;

  int bound = k1k2(sol, bl, br);
  if(bl.size() > 1) bound += k3(sol, bl); // no need to compute k3 when bl has one single element
  if(br.size() > 1) bound += k4(sol, br); // no need to compute k3 when br has one single element

  return bound;
}

////////////////////////////////////////////////////////////////////////////////
// Explores the solution tree by using branch and bound - best fit
void explore()
{
  while(sol_tree.size() > 0 && sol_tree.front().lower_bound < best_sol.lower_bound)
  {
    // If we've found a possible solution
    if(sol_tree.front().comp.size() == 0)
    {
      // If the solution is actually better
      if(sol_tree.front().lower_bound < best_sol.lower_bound) {
        // the solution is actually better
        update_solution(sol_tree.front());
      }

      // pop from heap
      std::pop_heap(sol_tree.begin(), sol_tree.end());
      sol_tree.pop_back();
    } else
    {
      // Increase number of explored nodes
      nexplored++;

      int st_size = sol_tree.size(), idx = -1, min = -1;

      if(sol_tree.front().lactive == (short)sol_tree.front().sol.size() - sol_tree.front().ractive) {
        idx = ++sol_tree.front().lactive-1; // insert on the left
      } else {
        idx = --sol_tree.front().ractive; // insert on the right
        min = (sol_tree.front().ractive == (short)sol_tree.front().sol.size()-1) ? sol_tree.front().sol[sol_tree.front().lactive-1] : -1;
      }

      // for each possible scene, creates a new solution with it and inserts it
      // into the solution tree if its lower bound allows
      for(short it=0; it < (short)sol_tree.front().comp.size(); it++) {
        short scene = sol_tree.front().comp[it];

        if(min < scene) { // This if breaks simetry of solutions
          // Creates the new partial solution candidate
          solution new_node(sol_tree.front());
          new_node.comp.erase(new_node.comp.begin()+it, new_node.comp.begin()+it+1);
          new_node.sol[idx] = scene;

          // std::cerr << new_node.sol.size() << std::endl;

          new_node.lower_bound = lower_bound(new_node);

          // Completes the partial solution candidate by using a greedy algorithm
          solution greedy(new_node);
          greedy_solution(greedy);

          // mature node condition
          if(new_node.lower_bound < greedy.lower_bound && new_node.lower_bound < best_sol.lower_bound) sol_tree.push_back(new_node);
          // If greedy is better than current, update best solution so far
          if(greedy.lower_bound < best_sol.lower_bound) update_solution(greedy);
        }
      }

      // pop from heap
      std::pop_heap(sol_tree.begin(), sol_tree.end());
      sol_tree.pop_back();

      // updates heap with freshly added nodes
      for(int it = std::max(st_size-1, 1); it < (int)sol_tree.size(); it++) {
        std::push_heap(sol_tree.begin(), sol_tree.begin()+it);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
