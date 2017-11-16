CC=g++
CXXFLAGS=-O3 -std=c++11
# CXXFLAGS=-O0 -g -Wall -std=c++11

all: bnb heur

bnb: bnb.cpp
	$(CC) $(CXXFLAGS) bnb.cpp -o bnb

heur: heur.cpp
	$(CC) $(CXXFLAGS) heur.cpp -o heur

pli-solver: pli-solver.c
	gcc -O3 pli-solver.c -lglpk -o pli-solver

clear:
	rm -f bnb heur pli-solver
