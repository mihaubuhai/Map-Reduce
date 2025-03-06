#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <vector>
#include <array>
#include <list>
#include "pthread.h"

namespace mapper {
    #define MAPPAIR std::pair<std::string, int>

    // Written just for me in case I forget how to use unordered_set XD
    // struct PairHash {
        // std::size_t operator()(const std::pair<std::string, int>& p) const {
            // return std::hash<std::string>()(p.first) ^ (std::hash<int>()(p.second) << 1);
        // }
    // };
    // typedef std::vector<std::unordered_set<MAPPAIR, PairHash>> MapperArr;

    // MapperArr is a vector of sets of pairs,
    //  pairs that contain a string (a word from a file)
    //  and the file index
    typedef std::vector<std::set<MAPPAIR>> MapperArr;

    void *mapper_func(void*);

    typedef struct {
        // The barrier where mappers and reducers stop for synchronization
        pthread_barrier_t *barrier;

        // The file ID
        int *file_idx;

        // The file that contains the names of the files to be processed
        std::ifstream *fin;

        // Mutex corresponding to "fin"
        pthread_mutex_t *mutex_fin;

        // The vector of sets of words populated by mappers
        MapperArr *map_arr;

        // To access the idx list in map_arr
        int idx;
    } marg, *MapperArg;

    std::string uniformize_string(std::string &in_string);
};

namespace reducer {
    #define PAIR std::pair<std::string, std::set<int>>
    #define LIST std::list<PAIR>
    
    // ReducerArr is an array of pairs:
    //      -> the first field is a list of pairs:
    //      -----> the first field is a string (a word)
    //      -----> the second field is a set of ints (a set of file indices)
    //      -> the second field is a mutex
    // ReducerArr has 26 entries, one for each letter of the English alphabet
    typedef std::array<std::pair<LIST, pthread_mutex_t>, 26> ReducerArr;

    typedef struct {
        // The barrier where mappers stop
        pthread_barrier_t *barrier;

        // The ID of a Reducer; to divide the work evenly
        int id;

        // The barrier where reducers stop before writing to files
        pthread_barrier_t *reducer_barrier;

        // The index pointing to the next line in MapperArg::map_arr to be processed
        int *list_idx;

        // Mutex for ReducerArg::list_idx
        pthread_mutex_t *mutex_lid;

        int nr_mappers;
        int nr_reducers;

        // The array of pairs, where each pair contains a list of words
        //  and a mutex to prevent concurrent writes to it
        ReducerArr *reducer_arr;

        // The vector of sets populated by mappers
        mapper::MapperArr *map_arr;
    } rarg, *ReducerArg;

    void *reducer_func(void*);
}

#define MARG ((mapper::MapperArg)arg)
#define RARG ((reducer::ReducerArg)arg)
