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

#include <bits/stdc++.h>

////////////////////////////////////////////////////////////////////////////////
// This class is the used class in the generic solution for the bnb algorithm
// It uses the score and evaluate functions to define how the bnb algorithm will
// walk through the solution tree
class solution
{
public:
  std::vector<bool> scenes; // 1 if scene in this solution, 0 otherwise
  std::vector<int> sol; // solution array
  int lactive, ractive; // end and start indices for left and right sets
  int lower_bound;      // solution's lower bound. It's the solution cost if lactive == ractive
  int score;            // solution's score

  solution() {}
  solution(int elems) : sol(elems, -1), scenes(elems, false), lactive(0), ractive(elems), lower_bound(0), score(0) {}
  solution(const std::vector<int>& sol, const std::vector<bool>& scenes, int lactive, int ractive, int lower_bound, int score) :
    sol(sol), scenes(scenes), lactive(lactive), ractive(ractive), lower_bound(lower_bound), score(score) {}
  solution(const solution& other) { *this = other; }

  // Copies a solution
  solution& operator=(const solution& other);

  // Indexer operator
  inline int& operator[](int index) { return this->sol[index]; }

  bool operator<(const solution& other);
};

bool solution::operator<(const solution& other)
{
  // if the score of the two are equal, prefer the one closest to the leafs
  if(this->score == other.score)
    return this->lactive < other.lactive;
  else
    return this->score < other.score;
}

solution& solution::operator=(const solution& other)
{
  this->lactive = other.lactive;
  this->ractive = other.ractive;
  this->lower_bound = other.lower_bound;
  this->score = other.score;

  this->sol = other.sol;
  this->scenes = other.scenes;

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// Auxiliary functions
template< typename T > std::ostream& operator<<(std::ostream& out, const std::vector<T>& v);
void print_and_exit(int signum=0);
void update_solution(const solution& new_node);

////////////////////////////////////////////////////////////////////////////////
// BnB functions
int lower_bound(solution& sol);
void explore();

////////////////////////////////////////////////////////////////////////////////
// Global variables
std::mutex sol_lock;  // Solution mutex
solution best_sol; // best solution so far

std::vector<std::vector<bool>> t; // t matrix
std::vector<int> costs, wdays; // cost array for each actor
int nscenes, nactors; // number of scenes and actors
int nexplored = 0; // Number of explored nodes

// Solutions tree. Each node is made of a solution and it's lower bound.
// In the leafs, the lower bound equals the cost of that solution
std::vector<solution> sol_tree;

////////////////////////////////////////////////////////////////////////////////
// Main function. Reads input and call other methods
int main(int argc, char **argv)
{
  // Signal handling
  signal(SIGINT, print_and_exit);

  // Lets read the input
  std::ifstream input(argv[1], std::ios_base::in);

  // Reads parameters (scenes, actors)
  input >> nscenes >> nactors;
  // Reads scenesXactors matrix
  t.resize(nactors);
  wdays.assign(nactors, 0);
  for(int i=0; i < nactors; i++) {
    t[i].resize(nscenes);
    for(int j=0; j < nscenes; j++) {
      bool isin;

      input >> isin;
      t[i][j] = isin;

      if(isin) wdays[i]++; // total number of working days per actor
    }
  }
  // Reads actors costs
  costs.resize(nactors);
  for(int i=0; i < nactors; i++) {
    int c;

    input >> c;
    costs[i] = c;
  }

  // Read is finished, lets run the algorithm

  // Lets start by setting a best solution at first
  // TODO change this part to heuristics ?
  best_sol.sol.resize(nscenes);
  best_sol.scenes.resize(nscenes);
  for(int i=0; i < nscenes; i++) {
    best_sol.sol[i] = i;
    best_sol.scenes[i] = true;
  }
  best_sol.lactive = (nscenes+1)/2;
  best_sol.ractive = (nscenes-1)/2;
  best_sol.lower_bound = lower_bound(best_sol);

  // std::cout << "First: " << best_sol.sol << " | " << best_sol.lower_bound << "|left: " << best_sol.lactive << "|right: " << best_sol.ractive << std::endl;

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

  // Computes number of actors in bl per scene
  for(int scene=0; scene < nscenes; scene++) {
    if(sol.scenes[scene] == false)
    {
      candidates.push_back(std::make_tuple(scene, std::vector<int>(), 0));

      for(int i=0; i < (int)bl.size(); i++) {
        int actor = bl[i];

        if(t[actor][scene]) {
          std::get<1>(candidates.back()).push_back(actor);
          std::get<2>(candidates.back()) += costs[actor];
        }
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
// TODO
int lower_bound(solution& sol)
{
  std::vector<int> bl, br;

  int bound = k1k2(sol, bl, br);
  bound += k3(sol, bl);
  bound += k4(sol, br);

  return bound;
}

////////////////////////////////////////////////////////////////////////////////
// TODO
int score(solution& sol)
{
  return sol.lower_bound;
}

////////////////////////////////////////////////////////////////////////////////

void explore()
{
  while(sol_tree.size() > 0)
  {
    // Pops head from the priority queue
    solution node = sol_tree.front();
    std::pop_heap(sol_tree.begin(), sol_tree.end());
    sol_tree.pop_back();

    // Number of explored nodes
    nexplored++;

    // std::cout << "exploring: " << node.sol << " | " << node.lower_bound << "|left: " << node.lactive << "|right: " << node.ractive << std::endl;

    // If we've found a possible solution
    if(node.lactive >= node.ractive) {
        // If the solution is actually better
        if(node.lower_bound < best_sol.lower_bound) {
          // the solution is actually better
          update_solution(node);
        }
    }
    // We are still on the search
    else {
      // Computes insertion index in the new solutions to be explored;
      int idx = (node.lactive == (int)node.sol.size() - node.ractive) ? ++node.lactive-1 : --node.ractive;

      // keeps tree size for later use
      int it = sol_tree.size();

      // for each possible scene, creates a new solution with it and inserts it
      // into the solution tree if its lower bound allows
      for(int sc=0; sc < nscenes; sc++) {
        if(node.scenes[sc] == false) {
          solution new_node(node);

          new_node.scenes[sc] = true;
          new_node.sol[idx] = sc;
          new_node.lower_bound = lower_bound(new_node);

          // mature node condition
          if(new_node.lower_bound < best_sol.lower_bound) {
            new_node.score = score(new_node);

            sol_tree.push_back(new_node);
          } else {
            // std::cout << "Discarding solution " << new_node.sol << "| lower_bound " << new_node.lower_bound << "| lactive " << new_node.lactive << "| ractive " << new_node.ractive << " | best_sol.lower " << best_sol.lower_bound << std::endl;
          }
        }
      }

      // updates heap with freshly added nodes
      for(it = std::max(it, 1); it <= (int)sol_tree.size(); it++) {
        std::push_heap(sol_tree.begin(), sol_tree.begin()+it);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Prints vector to output
template< typename T >
std::ostream& operator<<(std::ostream& out, const std::vector<T>& v)
{
  for(const auto& s: v) out << s << " ";

  return out;
}

////////////////////////////////////////////////////////////////////////////////
// prints solution and exits
void print_and_exit(int signum)
{
  // Acquires lock
  sol_lock.lock();
  // prints solution
  std::cout << best_sol.sol << std::endl << best_sol.lower_bound << std::endl;

  std::cout << "Number of explored nodes " << nexplored << std::endl;

  // exit
  exit(EXIT_SUCCESS);

  // should never reach this point
  sol_lock.unlock();
}

////////////////////////////////////////////////////////////////////////////////
// Updates the solution in a safe way with lockers !!!
void update_solution(const solution& newsol)
{
  sol_lock.lock();

  best_sol = newsol;

  sol_lock.unlock();
}

////////////////////////////////////////////////////////////////////////////////
