#ifndef COMMON_H
#define COMMON_H

////////////////////////////////////////////////////////////////////////////////

#include <bits/stdc++.h>

////////////////////////////////////////////////////////////////////////////////
// This class is the used class in the generic solution for the bnb algorithm
// It uses the lower_bound and evaluate functions to define how the bnb algorithm will
// walk through the solution tree
class solution
{
public:
  std::vector<int> sol; // solution array
  std::vector<int> comp; // scenes to be added to this solution

  int lactive, ractive; // end and start indices for left and right sets
  int lower_bound;      // solution's lower bound. It's the solution cost if lactive == ractive

  solution(int elems=0);
  solution(const solution& other) { *this = other; }

  // Copies a solution
  solution& operator=(const solution& other);

  // Indexer operator
  inline int operator[](int index) { return this->sol[index]; }

  bool operator<(const solution& other);
};

////////////////////////////////////////////////////////////////////////////////
// Auxiliary functions
template< typename T > std::ostream& operator<<(std::ostream& out, const std::vector<T>& v);
void read_input(char *filename);
void print_and_exit(int signum=0);
void update_solution(const solution& new_node);

////////////////////////////////////////////////////////////////////////////////
// Global variables
extern std::mutex sol_lock;  // Solution mutex
extern solution best_sol; // best solution so far

extern std::vector<std::vector<bool>> t; // t matrix
extern std::vector<int> costs, wdays; // cost array for each actor
extern int nscenes, nactors; // number of scenes and actors
extern int nexplored; // number of explored nodes on tree

// Necessary for branch and bound algorithm
// Solutions tree. Each node is made of a solution and it's lower bound.
// In the leafs, the lower bound equals the cost of that solution
extern std::vector<solution> sol_tree;

extern bool is_bnb;

////////////////////////////////////////////////////////////////////////////////

#endif /*! COMMON_HPP */

////////////////////////////////////////////////////////////////////////////////