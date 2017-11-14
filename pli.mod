/* declaracoes de conjuntos e parametros */

param n, integer, >=1;  /* quantidade de cenas */
param m, integer, >= 1; /* quantidade de atores */

/* matriz T */
set ATORES := {1..m}; /* conjunto de indices de atores */
set CENAS := {1..n};  /* conjunto de indices de dias de gravacao */
param T{i in ATORES, j in CENAS};

/* custo diário de espera */
param c{i in ATORES};

/* numero de cenas nas quais cada ator participa */
param s{i in ATORES}:=sum{j in CENAS} T[i,j];

/* ===> variaveis: */


/* ===> funcao objetivo */


/* ===> restricoes */


/* resolve problema */
solve;

/* ===> imprime solucao (n valores inteiros separados por espaco, onde
o j-esimo valor corresponde ao dia em que foi gravada a cena j) */


/* ===> imprime custo da solucao encontrada */


end;
