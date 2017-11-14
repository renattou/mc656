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
  inline int operator[](int index) { return this->sol[index]; }

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
int scenes, actors; // number of scenes and actors

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
  input >> scenes >> actors;
  // Reads scenesXactors matrix
  t.resize(actors);
  wdays.assign(actors, 0);
  for(int i=0; i < actors; i++) {
    t[i].resize(scenes);
    for(int j=0; j < scenes; j++) {
      bool isin;

      input >> isin;
      t[i][j] = isin;

      if(isin) wdays[i]++; // total number of working days per actor
    }
  }
  // Reads actors costs
  costs.resize(actors);
  for(int i=0; i < actors; i++) {
    int c;

    input >> c;
    costs[i] = c;
  }

  // Read is finished, lets run the algorithm

  // Lets start by setting a best solution at first
  // TODO change this part to heuristics ?
  best_sol.sol.resize(scenes);
  best_sol.scenes.resize(scenes);
  for(int i=0; i < scenes; i++) {
    best_sol.sol[i] = i;
    best_sol.scenes[i] = true;
  }
  best_sol.lactive = (scenes+1)/2;
  best_sol.ractive = (scenes-1)/2;
  best_sol.lower_bound = lower_bound(best_sol);

  // std::cout << "First: " << best_sol.sol << " | " << best_sol.lower_bound << "|left: " << best_sol.lactive << "|right: " << best_sol.ractive << std::endl;

  // Creates tree root with best solution so far
  sol_tree.push_back(solution(scenes));

  // Explores solution tree and updates best solution so far
  explore();

  // Exploration is finished, prints and exit
  print_and_exit();

  return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

int k1k2(solution& sol, std::vector<int>& sc)
{
  int lmost, rmost, rlmost, lrmost, partial;
  int cost = 0;

  for(int i=0; i < actors; i++)
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
    }
    else if(rmost != -1) {
      for(int j=rmost-1; j > rlmost; j--) partial += (1 - (int)t[i][sol.sol[j]]);
    }

    cost += partial * costs[i];
  }

  return cost;
}

int k3(solution& sol, std::vector<int>& sc)
{
  return 0;
}

int k4(solution& sol, std::vector<int>& sc)
{
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// TODO
int lower_bound(solution& sol)
{
  std::vector<int> sc(scenes);

  for(int i=0; i < scenes; i++) sc[i] = i;

  int bound = k1k2(sol, sc);
  bound += k3(sol, sc);
  bound += k4(sol, sc);

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
      for(int sc=0; sc < scenes; sc++) {
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
