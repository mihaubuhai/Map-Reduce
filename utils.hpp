#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <vector>
#include <array>
#include <list>
#include "pthread.h"

namespace mapper {
    #define MAPPAIR std::pair<std::string, int>

    // MapperArr este un vector de set-uri de perechi,
    //  perechi ce contin un string (un cuvant dintr-un fisier)
    //  si indicele fisierului
    typedef std::vector<std::set<MAPPAIR>> MapperArr;

    void *mapper_func(void*);

    typedef struct {
        // Bariera la care se opresc mapper-ii si reducer-ii pentru sincronizare
        pthread_barrier_t *barrier;

        // Id-ul unui fisier
        int *file_idx;

        // Fisierul care contine numele fisierelor de procesat
        std::ifstream *fin;

        // Mutex-ul corespunzator "fin"
        pthread_mutex_t *mutex_fin;

        // Vectorul de set-uri de cuvinte populate de mapper-i
        MapperArr *map_arr;

        // Ca sa acceseze a idx lista din map_arr
        int idx;
    } marg, *MapperArg;

    std::string uniformize_string(std::string &in_string);
};

namespace reducer {
    #define PAIR std::pair<std::string, std::set<int>>
    #define LIST std::list<PAIR>
    
    // ReducerArr este un array de perechi:
    //      -> primul camp este o lista de perechi:
    //      -----> primul camp este un string (un cuvant)
    //      -----> al doilea camp este un set de ints (un set de indici de fisiere)
    //      -> al doilea camp este un mutex
    // ReducerArr are 26 de intrari, o intrare pentru fiecare litera din alfabetul englez
    typedef std::array<std::pair<LIST, pthread_mutex_t>, 26> ReducerArr;

    typedef struct {
        // Bariera la care se opresc si Mapper-ii
        pthread_barrier_t *barrier;

        // Id-ul unui Reducer; pentru a imparti in mod egal munca
        int id;

        // Bariera la care se opresc Reducer-ii inainte sa scrie in fisiere
        pthread_barrier_t *reducer_barrier;

        // Indice care puncteaza catre urmatoarea linie din MapperArg::map_arr de procesat
        int *list_idx;

        // Mutex pentru ReducerArg::list_idx
        pthread_mutex_t *mutex_lid;

        int nr_mappers;
        int nr_reducers;

        // Array-ul de perechi, pereche ce contine o lista de cuvinte
        //  si un mutex pentru a preveni scrieri concomitente in ea 
        ReducerArr *reducer_arr;

        // Vector-ul de set-uri populate de mapper-i
        mapper::MapperArr *map_arr;
    } rarg, *ReducerArg;

    void *reducer_func(void*);
}

#define MARG ((mapper::MapperArg)arg)
#define RARG ((reducer::ReducerArg)arg)
