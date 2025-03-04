#include "utils.hpp"

int main(int argc, char **argv) {
    int nr_mappers = atoi(argv[1]);
    int nr_reducers = atoi(argv[2]);
    int nr_threads = nr_mappers + nr_reducers;
    pthread_t threads[nr_threads];

    int nr_files = 0;
    int file_idx = 1;   // Indicele fisierului folosit de mapper-i

    std::ifstream fin(argv[3]);
    fin >> nr_files;

    // Mutex pentru mapper pentru a accesa sigur un fisier
    pthread_mutex_t mutex_fin;
    pthread_mutex_init(&mutex_fin, NULL);

    // Bariera la care se opresc mapper-ii inainte de finalizare
    //  si reducer-ii inainte sa inceapa sa lucreze
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, nr_threads);

    // Array-ul de liste pe care le completeaza Mapper-i
    mapper::MapperArr marr(nr_mappers);

    // Array-ul de liste cu cuvinte scris de Reducer-i pentru fisierele out
    reducer::ReducerArr rarr;
    for (auto &p : rarr) {
        pthread_mutex_init(&(p.second), NULL);
    }

    // Variabila folosita pentru accesarea unei linii din MapperArg::map_arr
    int list_idx = 0;

    // Mutex folosit de reducer-i pentru variabila "list_idx"
    pthread_mutex_t mutex_lid;
    pthread_mutex_init(&mutex_lid, NULL);

    // Bariera la care se opresc reducer-ii inainte de a scrie in fisiere
    pthread_barrier_t reducer_barrier;
    pthread_barrier_init(&reducer_barrier, NULL, nr_reducers);
    
    // Un pointer care va puncta catre fie mapper_func(), fie reducer_func()
    void *(*f)(void *);
    // Argumentul pasat functiei f de mai sus
    void *arg;

    for (int i = 0; i < nr_threads; ++i) {
        if (i < nr_mappers) { // Cream un mapper
            f = mapper::mapper_func;
            arg = calloc(1, sizeof(mapper::marg));
            MARG->barrier = &barrier;
            MARG->file_idx = &file_idx;
            MARG->fin = &fin;
            MARG->idx = i;
            MARG->map_arr = &marr;
            MARG->mutex_fin = &mutex_fin;
        } else {    // Cream un reducer
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
            std::cout << "Eroare la crearea unui thread!\n";
            exit(-1);
        }
    }

    for (int i = 0; i < nr_threads; ++i) {
        int rez = pthread_join(threads[i], NULL);

        if (rez) {
            std::cout << "Eroare la join pe thread\n";
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