CC=g++
CXXFLAGS=-O3 -std=c++11
# CXXFLAGS=-O0 -g -Wall -std=c++11

all: bnb heur

bnb: bnb.cpp common.cpp metaheuristic.cpp
	$(CC) $(CXXFLAGS) bnb.cpp common.cpp metaheuristic.cpp -o bnb

heur: heur.cpp common.cpp metaheuristic.cpp
	$(CC) $(CXXFLAGS) heur.cpp common.cpp metaheuristic.cpp -o heur

pli-solver: pli-solver.c
	gcc -O3 pli-solver.c -lglpk -o pli-solver

pack:
	tar -zcvf ra118827-ra118557.tar.gz *.hpp *.cpp relatorio/relatorio.pdf pli.mod Makefile 

clear:
	rm -f bnb heur pli-solver
