#include "utils.hpp"

int main(int argc, char **argv) {
    int nr_mappers = atoi(argv[1]);
    int nr_reducers = atoi(argv[2]);
    int nr_threads = nr_mappers + nr_reducers;
    pthread_t threads[nr_threads];

    int nr_files = 0;
    int file_idx = 1;   // Index of the file used by the mappers

    std::ifstream fin(argv[3]);
    fin >> nr_files;

    // Mutex for mappers to safely access a file
    pthread_mutex_t mutex_fin;
    pthread_mutex_init(&mutex_fin, NULL);

    // Barrier where mappers stop before finishing
    // and reducers stop before starting their work
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, nr_threads);

    // Array of lists that the mappers populate
    mapper::MapperArr marr(nr_mappers);

    // Array of lists with words written by the reducers for the output files
    reducer::ReducerArr rarr;
    for (auto &p : rarr) {
        pthread_mutex_init(&(p.second), NULL);
    }

    // Variable used to access a line from MapperArg::map_arr
    int list_idx = 0;

    // Mutex used by reducers for the "list_idx" variable
    pthread_mutex_t mutex_lid;
    pthread_mutex_init(&mutex_lid, NULL);

    // Barrier where reducers stop before writing to files
    pthread_barrier_t reducer_barrier;
    pthread_barrier_init(&reducer_barrier, NULL, nr_reducers);
    
    // A pointer that will point to either mapper_func() or reducer_func()
    void *(*f)(void *);
    // Argument passed to the function f above
    void *arg;

    for (int i = 0; i < nr_threads; ++i) {
        if (i < nr_mappers) { // Create a mapper
            f = mapper::mapper_func;
            arg = calloc(1, sizeof(mapper::marg));
            MARG->barrier = &barrier;
            MARG->file_idx = &file_idx;
            MARG->fin = &fin;
            MARG->idx = i;
            MARG->map_arr = &marr;
            MARG->mutex_fin = &mutex_fin;
        } else {    // Create a reducer
            f = reducer::reducer_func;
            arg = calloc(1, sizeof(reducer::rarg));
            RARG->barrier = &barrier;
            RARG->id = i - nr_mappers;
            RARG->list_idx = &list_idx;
            RARG->map_arr = &marr;
            RARG->mutex_lid = &mutex_lid;
            RARG->nr_mappers = nr_mappers;
            RARG->nr_reducers = nr_reducers;
            RARG->reducer_arr = &rarr;
            RARG->reducer_barrier = &reducer_barrier;
        }

        int rez = pthread_create(threads + i, NULL, f, arg);

        if (rez) {
            std::cout << "Error creating a thread!\n";
            exit(-1);
        }
    }

    for (int i = 0; i < nr_threads; ++i) {
        int rez = pthread_join(threads[i], NULL);

        if (rez) {
            std::cout << "Error joining a thread\n";
            exit(-1);
        }
    }

    fin.close();
    pthread_mutex_destroy(&mutex_fin);
    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex_lid);
    pthread_barrier_destroy(&reducer_barrier);
    for (int i = 0; i < 26; ++i) {
        pthread_mutex_destroy(&rarr[i].second);
    }

}
