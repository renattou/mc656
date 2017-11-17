#include "common.hpp"
#include "simmulated_anneling.hpp"

////////////////////////////////////////////////////////////////////////////////

const float TIMEOUT = 30000; // Time in which temperature goes to zero

////////////////////////////////////////////////////////////////////////////////
// Main function. Reads input and call other methods
int main(int argc, char **argv)
{
  // Signal handling
  signal(SIGINT, print_and_exit);

  // Reads from input file
  read_input(argv[1]);

  // Runs simmulated anneling until timeout
  simmulated_anneling(TIMEOUT);

  // Prints and exit
  print_and_exit();

  return EXIT_SUCCESS;
}
