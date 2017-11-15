#ifndef COMMON_H
#define COMMON_H

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
extern int scenes, actors; // number of scenes and actors

////////////////////////////////////////////////////////////////////////////////

#endif /*! COMMON_HPP */
