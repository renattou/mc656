////////////////////////////////////////////////////////////////////////////////

#include "common.hpp"
#include "metaheuristic.hpp"

////////////////////////////////////////////////////////////////////////////////

const float TIMEOUT = 30000; // Time in which the algorithm should finish

////////////////////////////////////////////////////////////////////////////////
// Main function. Reads input and call other methods
int main(int argc, char **argv)
{
  // Signal handling
  signal(SIGINT, print_and_exit);

  // Reads from input file
  read_input(argv[1]);

  // Runs genetic algorithm until timeout
  genetic_algorithm(TIMEOUT);

  // Prints and exit
  print_and_exit();

  return EXIT_SUCCESS;
}
