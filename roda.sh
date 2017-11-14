#!/bin/bash

# Parametros para interrupcao do programa
TLIM_BNB=180  # Tempo de execucao do branch and bound
TLIM_HEUR=30  # Tempo de execucao da heuristica
TTOL=5        # Tolerancia final

dir_codigo="$1"
dir_instancias="$2"
arq_saida="$3"
alg="$4"

# Verifica parametros de entrada
if [ -z "$dir_codigo" ]
then
    echo "Deve ser fornecido o diretorio de codigo"
    echo "Uso: $0 <dir-codigo> <dir-instancias> <arq-saida> <bnb | heur | pli>"
    exit
fi
if [ -z "$dir_instancias" ]
then
    echo "Deve ser fornecido o diretorio de instancias"
    echo "Uso: $0 <dir-codigo> <dir-instancias> <arq-saida> <bnb | heur | pli>"
    exit
fi
if [ -z "$arq_saida" ]
then
    echo "Deve ser fornecido o nome do arquivo de saida"
    echo "Uso: $0 <dir-codigo> <dir-instancias> <arq-saida> <bnb | heur | pli>"
    exit
fi
if [ -z "$alg" ]
then
    echo "Deve ser fornecido o algoritmo a ser executado"
    echo "Uso: $0 <dir-codigo> <dir-instancias> <arq-saida> <bnb | heur | pli>"
    exit
fi

# Verifica se o diretorio de codigo existe
[[ $dir_codigo != */ ]] && dir_codigo="$dir_codigo"/
if [ ! -d "$dir_codigo" ]
then
    echo "Diretorio de codigo nao encontrado"
    exit
fi

# Guarda diretorio original
dir_original=`pwd`
[[ $dir_original != */ ]] && dir_original="$dir_original"/

# Verifica se o diretorio de instancias existe
[[ $dir_instancias != */ ]] && dir_instancias="$dir_instancias"/
if [ ! -d "$dir_instancias" ]
then
    echo "Diretorio de instancias nao encontrado"
    exit
fi

# Verifica se consegue escrever no arquivo de saida
touch "$arq_saida" 2> /dev/null
if [[ $? != 0 ]]
then
    echo "Nao foi possivel escrever no arquivo de saida"
    exit
fi

# Verifica o algoritmo escolhido
if [ "$alg" != "bnb" ] && [ "$alg" != "heur" ] && [ "$alg" != "pli" ]
then
    echo "O algoritmo especificado deve ser 'bnb', 'heur' ou 'pli'"
    exit
fi

# Muda para diretorio de codigo
cd "$dir_codigo"

# Compila programa
if [ "$alg" != "pli" ]
then
    if [ -f "Makefile" ]
    then
        # Compila com Makefile
        make
    else
        # Procura arquivos com nome padrao (definidos na pagina da disciplina)
        if [ -f "$alg.c" ]
        then
            gcc -std=gnu11 -O3 "$alg.c" -lm -o "$alg"
        elif [ -f "$alg.cpp" ]
        then
            g++ -std=gnu++03 -O3 "$alg.cpp" -o "$alg"
        else
            (>&2 echo "Nenhum Makefile nem arquivo $alg.c ou $alg.cpp encontrado")
            (>&2 echo "Abortando experimentos")
            exit
        fi
    fi
    # Verifica se o executavel foi criado corretamente
    if [ ! -f "$alg" ]
    then
        (>&2 echo "Compilacao falhou")
        (>&2 echo "Executavel '$alg' nao foi encontrado")
        (>&2 echo "Abortando experimentos")
        exit
    fi
fi

echo "Resultados: $alg" >> "$dir_original$arq_saida"
if [ "$alg" = "bnb" ]
then
    echo "Instancia;Solucao;Custo;Lim. Inf.;Nos;Tempo" >> "$dir_original$arq_saida"
    for inst in `ls "$dir_original$dir_instancias"`
    do
        if [[ $inst != *.txt ]]
        then
            continue
        fi
        saida=`/usr/bin/time -o /dev/stdout -a -f %e --quiet timeout -s SIGINT --preserve-status -k $TTOL $TLIM_BNB ./bnb "$dir_original$dir_instancias$inst" | tee /dev/tty | tail -5`
        num_linhas=`echo "$saida" | wc -l`
        if [ "$num_linhas" != 5 ]
        then
            (>&2 echo "Formato incorreto de saida para a instancia $inst")
            echo "$inst;erro;erro;erro;erro;erro" >> "$dir_original$arq_saida"
        else
            sol=`echo "$saida" | head -1`
            custo=`echo "$saida" | head -2 | tail -1`
            dual=`echo "$saida" | head -3 | tail -1`
            nos=`echo "$saida" | head -4 | tail -1`
            tempo=`echo "$saida" | tail -1`
            echo "$inst;$sol;$custo;$dual;$nos;$tempo" >> "$dir_original$arq_saida"
        fi
    done
elif [ "$alg" = "heur" ]
then
    echo "Instancia;Solucao;Custo;Tempo" >> "$dir_original$arq_saida"
    for inst in `ls "$dir_original$dir_instancias"`
    do
        if [[ $inst != *.txt ]]
        then
            continue
        fi
        saida=`/usr/bin/time -o /dev/stdout -a -f %e --quiet timeout -s SIGINT --preserve-status -k $TTOL $TLIM_HEUR ./heur "$dir_original$dir_instancias$inst" | tee /dev/tty | tail -3`
        num_linhas=`echo "$saida" | wc -l`
        if [ "$num_linhas" != 3 ]
        then
            (>&2 echo "Formato incorreto de saida para a instancia $inst")
            echo "$inst;erro;erro;erro" >> "$dir_original$arq_saida"
        else
            sol=`echo "$saida" | head -1`
            custo=`echo "$saida" | head -2 | tail -1`
            tempo=`echo "$saida" | tail -1`
            echo "$inst;$sol;$custo;$tempo" >> "$dir_original$arq_saida"
        fi
    done
elif [ "$alg" = "pli" ]
then
    echo "Instancia;Solucao;Custo;Lim. Inf.;Nos;Tempo" >> "$dir_original$arq_saida"
    for inst in `ls "$dir_original$dir_instancias"`
    do
        if [[ $inst != *.dat ]]
        then
            continue
        fi
        saida=`/usr/bin/time -o /dev/stdout -a -f %e --quiet ./pli-solver "$dir_original$dir_instancias$inst" | tee /dev/tty | tail -6`
        num_linhas=`echo "$saida" | wc -l`
        if [ "$num_linhas" != 6 ]
        then
            (>&2 echo "Formato incorreto de saida para a instancia $inst")
            echo "$inst;erro;erro;erro;erro;erro" >> "$dir_original$arq_saida"
        else
            sol=`echo "$saida" | head -1`
            custo=`echo "$saida" | head -2 | tail -1`
            dual=`echo "$saida" | head -4 | tail -1`
            nos=`echo "$saida" | head -5 | tail -1`
            tempo=`echo "$saida" | tail -1`
            echo "$inst;$sol;$custo;$dual;$nos;$tempo" >> "$dir_original$arq_saida"
        fi
    done
fi

# Apaga arquivos compilados
rm -f *.o bnb heur
