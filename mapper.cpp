#include "utils.hpp"

std::string mapper::uniformize_string(std::string &in_string) {
    std::string rez;

    for (char c : in_string) {
        if (std::isalpha(c)) {
            rez += std::tolower(c);
        }
    }

    return rez;
}

// Functia pe care o executa un mapper
void *mapper::mapper_func(void *arg) {
    int file_nr = 0;
    int idx = MARG->idx;
    int &file_idx = *MARG->file_idx;
    
    auto &fin = *MARG->fin;

    // Set-ul de cuvinte asociat mapper-ului cu id-ul "idx"
    auto &set = (*(MARG->map_arr))[idx];

    std::string file_name;
    std::string word;
    while (1) {
        pthread_mutex_lock(MARG->mutex_fin);
        if (fin.peek() != EOF) {   // Putem lucra pe un fisier
            file_nr = (file_idx++);

            fin >> file_name;
        } else {    // Nu mai exista fisiere de prelucrat
            pthread_mutex_unlock(MARG->mutex_fin);
            break;
        }
        pthread_mutex_unlock(MARG->mutex_fin);

        // Deschidem fisierul si il citim cuvant cu cuvant
        std::ifstream temp_fin(file_name);
        while (temp_fin.peek() != EOF) {
            temp_fin >> word;

            if (word.size() < 1) {
                continue;
            }
            word = uniformize_string(word);

            set.insert({word, file_nr});
        }
        temp_fin.close();
    }

    // Asteptam executia tuturor mapper-ilor
    pthread_barrier_wait(MARG->barrier);
    free(arg);
    return NULL;
}