#include "common.hpp"

////////////////////////////////////////////////////////////////////////////////
// Operator overrides for solution class
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
// Global variables
std::mutex sol_lock;  // Solution mutex
solution best_sol; // best solution so far

std::vector<std::vector<bool>> t; // t matrix
std::vector<int> costs, wdays; // cost array for each actor
int scenes, actors; // number of scenes and actors

////////////////////////////////////////////////////////////////////////////////
// Prints vector to output
template< typename T >
std::ostream& operator<<(std::ostream& out, const std::vector<T>& v)
{
  for(const auto& s: v) out << s << " ";

  return out;
}

void read_input(char *filename)
{
  // Open input file
  std::ifstream input(filename, std::ios_base::in);

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
}

////////////////////////////////////////////////////////////////////////////////
// Prints solution and exits
void print_and_exit(int signum)
{
  // Acquires lock
  sol_lock.lock();

  // Prints solution
  std::cout << best_sol.sol << std::endl << best_sol.lower_bound << std::endl;

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
