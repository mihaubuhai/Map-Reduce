# Objective

```
    This project implements the Map Reduce paradigm in C++.
    The project creates {a..z}.txt files, where all words from the given files  
are stored along with their corresponding file index. Each word is stored in  
the file named after the first character of the word. (e.g., 'hello' will be stored in h.txt).
```

# Use

```
    make && ./mapred <nr_mappers> <nr_reducers> <file>, where:
        -> nr_mappers: total number of mappers (which are threads)
        -> nr_reducers: total number of reducers (which are threads)
        -> file: the input file, of the form:
                            X
                            PATH_TO_FILE_1
                            PATH_TO_FILE_2
                            ...
            ,where: X - number of files, PATH_TO_FILE_i - absolute or relative path to the file of interest

    ! All parameters are a must !
```

# Implementation

```
    Mappers take a file to process from the input file given as a parameter.  
    Access to this file is controlled using a mutex (mapper::mutex_fin), relying on
the index mapper::MapperArg->file_idx, which is incremented each time a Mapper retrieves a filename.  

    Mappers write pairs of {word, file_id} into a std::set, the reason being to
avoid storing duplicates. These sets are stored in a std::vector, resized to match the number of
Mappers running in parallel. Naturally, each Mapper will stop at a barrier immediately after
completing its work to ensure all sets are fully populated.  

    Reducers work on equal portions of each std::set written by the Mappers.
Specifically, a Reducer iterates through all sets created by the Mappers and, based on the ID
it was assigned in the main function, takes ownership of its designated portion and processes it.  

    The initial task of the Reducers is to populate a std::array of lists containing pairs
in the format (word, std::set<int>) with data written by the Mappers.
The reason for using a std::array of lists is that each list corresponds to a letter of the
English alphabet, and lists are used because the exact number of words starting with a given letter
is unknown and unimportant. Once this procedure is completed by all Reducers, the array will contain
all words and the file IDs in which they appear, categorized by the first letter of the word.  

    The second step for the Reducers is to sort these word lists: first, in descending order based on
the size of the sets, and in case of equality, lexicographically.
Naturally, Reducers will stop at a barrier before this step to ensure the array of lists is fully populated.  

    Immediately after sorting a list, a Reducer will create the corresponding file and write the sorted words into it.
```
