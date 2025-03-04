#include "utils.hpp"

// Functia pe care o executa un reducer
void *reducer::reducer_func(void *arg) {
    pthread_barrier_wait(RARG->barrier);

    int id = RARG->id;
    int nr_mappers = RARG->nr_mappers;
    int nr_reducers = RARG->nr_reducers;

    // Vom imparti fiecare lista a mapper-ilor in mod egal intre Reducer-i
    for (int i = 0; i < nr_mappers; ++i) {
        // Iteram prin array-ul de set-uri
        auto &set = (*(RARG->map_arr))[i];
        double size = (double)set.size();

        // Calculam indicii pentru regiunea de lucru a unui reducer,
        //  indici folositi pentru a accesa set-ul unui mapper
        int start_step = (int)(id * size / nr_reducers);
        int k = (int)((id + 1) * size / nr_reducers);
        int end_step = k < size ? k : size;

        // Iteratorul de inceput al set-ului
        auto s_itr = set.begin();
        
        // Iteratorul final al set-ului
        auto end = std::next(s_itr, end_step);
        for (auto it = std::next(s_itr, start_step); it != end; ++it) {
            // boolean folosit pentru a sti daca sa adaugam o intrare noua in set-ul de cuvinte
            bool found = false;
            auto &word = (*it).first;
            // Indicele cu care vom accesa lista de cuvinte din array-ul reducer-ilor
            int arr_idx = word[0] - 'a';

            if (arr_idx < 0 || arr_idx > 26) {
                continue;
            }

            // Mutex-ul aferent unei liste de cuvinte din array-ul reducer-ilor
            auto *mutex = &(*(RARG->reducer_arr))[arr_idx].second;

            // Lista de cuvinte din array-ul reducer-ilor
            auto &red_arr_l = (*(RARG->reducer_arr))[arr_idx].first;

            pthread_mutex_lock(mutex);
            // Iteram prin lista si cautam cuvantul "word"
            for (auto &el : red_arr_l) {
                if (el.first == word) { // Adaugam noul fisier
                    el.second.insert((*it).second);
                    found = true;
                    break;
                }
            }

            // Cuvantul nu se gaseste in lista, il adaugam
            if (!found) {
                std::set<int> s { (*it).second };
                (red_arr_l).push_back({word, s});
            }
            pthread_mutex_unlock(mutex);
        }
    }

    // Asteptam ca toti reducer-ii sa isi termine executia
    pthread_barrier_wait(RARG->reducer_barrier);

    // Calculam indicii care delimiteaza zona de lucru a unui reducer anume,
    //  indici cu care se vom accesa array-ul de liste de cuvinte "reducer::reducer_arr"
    int start_step = (int)(id * 26. / nr_reducers);
    int k = (int)((id + 1) * 26. / nr_reducers);
    int end_step = k < 26 ? k : 26;

    char s[6] = {};
    for (int i = start_step; i < end_step; ++i) {
        sprintf(s, "%c.txt", i + 'a');

        auto &l = (*(RARG->reducer_arr))[i].first;
        l.sort([](auto& a, auto& b) {
            // Comparam dupa dimensiunea set-ului de indici de fisiere
            if (a.second.size() != b.second.size()) {
                return a.second.size() > b.second.size();
            }

            // Set-uri de dimensiuni egale, atunci comparam lexicografic cuvintele
            return a.first < b.first;
        }); 

        // Deschidem fisierul si scriem in el
        std::ofstream fout(s);
        for (auto &p : l) {
            fout << p.first << ":[";
            
            // last_el este ultimul element; il scriem explicit pentru a formata frumos output-ul
            auto last_el = std::next(p.second.begin(), p.second.size() - 1);
            for (auto it = p.second.begin(); it != last_el; ++it) {
                fout << *it << " ";
            }
            fout << *last_el << "]\n";
        }
        fout.close();
    }

    free(arg);
    return NULL;
}