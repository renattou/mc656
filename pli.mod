/* declaracoes de conjuntos e parametros */

param n, integer, >=1;  /* quantidade de cenas */
param m, integer, >= 1; /* quantidade de atores */

/* matriz T */
set ATORES := {1..m}; /* conjunto de indices de atores */
set CENAS := {1..n};  /* conjunto de indices das cenas */
set DIAS := {1..n}; /* conjunto de indices dos dias de gravacao */
param T{i in ATORES, j in CENAS};

/* custo diário de espera */
param c{i in ATORES};

/* numero de cenas nas quais cada ator participa */
param s{i in ATORES}:=sum{j in CENAS} T[i,j];

/* ===> variaveis: */
var p{j in CENAS, l in DIAS} binary; /* cena j esta no dia l */
var x{i in ATORES, l in DIAS} binary; /* ator i esta no dia l */
var e{i in ATORES} >= 1 <= n; /* dia em que o ator i comeca a trabalhar */
var d{i in ATORES} >= 1 <= n; /* dia em que o ator i termina de trabalhar */
var h{i in ATORES} >= 0; /* dias de espera do ator i */

/* ===> funcao objetivo */
minimize cost: sum{i in ATORES} c[i] * h[i];

/* ===> restricoes */
s.t. cena_dia{l in DIAS}: sum{j in CENAS} p[j, l] = 1; /* Uma cena por dia */
s.t. dia_cena{j in CENAS}: sum{l in DIAS} p[j, l] = 1; /* Um dia por cena */
s.t. actor_cene_day{i in ATORES, l in DIAS}: sum{j in CENAS} T[i, j] * p[j, l] = x[i, l]; /* ator i esta em um dia l, se estiver em uma cena j desse dia */
s.t. first_day{i in ATORES, l in DIAS}: e[i] <= n + (l-n)*x[i,l]; /* primeiro dia do ator i (faz o minimo de todos os indices) */
s.t. last_day{i in ATORES, l in DIAS}: d[i] >= l*x[i,l]; /* ultimo dia do ator i (faz o maximo de todos os indices) */
s.t. wait{i in ATORES}: d[i] - e[i] + 1 - s[i] = h[i]; /* dias ociosos do ator i */
s.t. sem_simetria {j in CENAS}: sum{i in {1..j}} p[i,n] <= 1 - p[j,1]; 

/* resolve problema */
solve;

/* ===> imprime solucao (n valores inteiros separados por espaco, onde
o j-esimo valor corresponde ao dia em que foi gravada a cena j) */
param perm{l in DIAS}:= sum{j in CENAS} j*p[j, l]; /* permutacao das cenas */
printf {l in DIAS} '%d ', perm[l];

/* ===> imprime custo da solucao encontrada */
printf '\n%d\n', cost;

end;
