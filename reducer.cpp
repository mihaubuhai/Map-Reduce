#include "utils.hpp"

// The function executed by a reducer
void *reducer::reducer_func(void *arg) {
    pthread_barrier_wait(RARG->barrier);

    int id = RARG->id;
    int nr_mappers = RARG->nr_mappers;
    int nr_reducers = RARG->nr_reducers;

    // We will divide each mapper's list equally among the reducers
    for (int i = 0; i < nr_mappers; ++i) {
        // Iterate through the array of sets
        auto &set = (*(RARG->map_arr))[i];
        double size = (double)set.size();

        // Calculate the indices for the reducer's working region,
        // indices used to access a mapper's set
        int start_step = (int)(id * size / nr_reducers);
        int k = (int)((id + 1) * size / nr_reducers);
        int end_step = k < size ? k : size;

        // Iterator pointing to the start of the set
        auto s_itr = set.begin();
        
        // Iterator pointing to the end of the working region in the set
        auto end = std::next(s_itr, end_step);
        for (auto it = std::next(s_itr, start_step); it != end; ++it) {
            // Boolean used to check whether to add a new entry in the word set
            bool found = false;
            auto &word = (*it).first;
            // Index used to access the word list in the reducers' array
            int arr_idx = word[0] - 'a';

            if (arr_idx < 0 || arr_idx > 26) {
                continue;
            }

            // Mutex associated with a word list in the reducers' array
            auto *mutex = &(*(RARG->reducer_arr))[arr_idx].second;

            // The word list in the reducers' array
            auto &red_arr_l = (*(RARG->reducer_arr))[arr_idx].first;

            pthread_mutex_lock(mutex);
            // Iterate through the list and search for the word "word"
            for (auto &el : red_arr_l) {
                if (el.first == word) { // Add the new file
                    el.second.insert((*it).second);
                    found = true;
                    break;
                }
            }

            // If the word is not found in the list, add it
            if (!found) {
                std::set<int> s { (*it).second };
                (red_arr_l).push_back({word, s});
            }
            pthread_mutex_unlock(mutex);
        }
    }

    // Wait for all reducers to finish execution
    pthread_barrier_wait(RARG->reducer_barrier);

    // Calculate the indices defining a specific reducer's working region,
    // indices used to access the array of word lists "reducer::reducer_arr"
    int start_step = (int)(id * 26. / nr_reducers);
    int k = (int)((id + 1) * 26. / nr_reducers);
    int end_step = k < 26 ? k : 26;

    char s[6] = {};
    for (int i = start_step; i < end_step; ++i) {
        sprintf(s, "%c.txt", i + 'a');

        auto &l = (*(RARG->reducer_arr))[i].first;
        l.sort([](auto& a, auto& b) {
            // Compare based on the size of the set of file indices
            if (a.second.size() != b.second.size()) {
                return a.second.size() > b.second.size();
            }

            // If sets are of equal size, compare words lexicographically
            return a.first < b.first;
        }); 

        // Open the file and write to it
        std::ofstream fout(s);
        for (auto &p : l) {
            fout << p.first << ":[";

            // last_el is the last element; we write it explicitly for proper output formatting
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
