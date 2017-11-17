////////////////////////////////////////////////////////////////////////////////

#include "common.hpp"

////////////////////////////////////////////////////////////////////////////////

solution::solution(int elems) :
  sol(elems, -1), comp(elems, 0), lactive(0), ractive(elems), lower_bound(0)
{
  for(int i=0; i < elems; i++) this->comp[i] = i;
}

////////////////////////////////////////////////////////////////////////////////
// Operator overrides for solution class
bool solution::operator<(const solution& other)
{
  // if the lower_bound of the two are equal, prefer the one closest to the leafs
  if(this->lower_bound == other.lower_bound)
    return this->lactive < other.lactive;
  else
    return this->lower_bound > other.lower_bound;
}

solution& solution::operator=(const solution& other)
{
  this->lactive = other.lactive;
  this->ractive = other.ractive;
  this->lower_bound = other.lower_bound;

  this->sol = other.sol;
  this->comp = other.comp;

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// Global variables
std::mutex sol_lock;  // Solution mutex
solution best_sol; // best solution so far

std::vector<std::vector<bool>> t; // t matrix
std::vector<int> costs, wdays; // cost array for each actor
int nscenes, nactors; // number of scenes and actors
long long unsigned nexplored = 0; // number of explored nodes

std::vector<solution> sol_tree; // solution tree (min-heap)

bool is_bnb = false;

////////////////////////////////////////////////////////////////////////////////

void read_input(char *filename)
{
  // Open input file
  std::ifstream input(filename, std::ios_base::in);

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
}

////////////////////////////////////////////////////////////////////////////////
// Prints solution and exits
void print_and_exit(int signum)
{
  // Acquires lock
  sol_lock.lock();

  // Prints solution
  std::cout << best_sol.sol << std::endl << best_sol.lower_bound << std::endl;

  if(is_bnb) {
    int dual = sol_tree.size() > 0 ? sol_tree.front().lower_bound : best_sol.lower_bound;

    std::cout << std::min(best_sol.lower_bound, dual) << std::endl;
    std::cout << nexplored << std::endl;
  }

  // Exits
  exit(EXIT_SUCCESS);

  // Should never reach this point
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
