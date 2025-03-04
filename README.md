##Idee rezolvare tema
    Mapper-ii isi iau cate un fisier de prelucrat din fisierul dat
  ca parametru, pe care se foloseste un mutex (mapper::mutex_fin),
  folosindu-se de indicele mapper::MapperArg->file_idx, incrementat
  de fiecare data cand un Mapper a luat numele unui fisier.
    Mapper-ii scriu perechile de {cuvant, id_fisier} intr-un std::set,
  motivul fiind nestocarea de duplicate. Set-urile se gasesc intr-un
  std::vector, redimensionat la numarul de Mapper care ruleaza in paralel.
  Natural, fiecare Mapper se va opri la o bariera imediat dupa terminarea
  lucrului sau, pentru a asigura popularea completa a tuturor set-urilor.

    Reducer-ii lucreaza pe portiuni egale din fiecare std::set scris de
  Mapper-i, adica un Reducer itereaza prin toate set-urile scrise de
  Mapper-i, iar in functie de id-ul sau cu care a fost initializat in
  functia main, isi ia in primire portiunea din set si lucreaza pe ea.
    Lucrul initial al Reducer-ilor este de a popula un std::array de liste
  de perechi de forma (cuvant, std::set<int>) cu datele scrise de Mapper-i.
  Motivul folosirii unui std::array de liste este acela ca o lista corespunde
  unei litere din alfabetul englez, iar liste pentru ca nu se cunoaste si nu
  este important numarul exact de cuvinte care incep cu acea litera. Astfel,
  array-ul dupa finalizarea acestei proceduri de catre Reducer-i, va contine
  toate cuvintele si id-urile fisierelor in care se gasesc acestea, categorisite
  in functie de prima litera a cuvantului.
    A doua procedura pe care Reducer-ii o urmeaza este sortarea acestor liste
  de cuvinte, intai descrescator dupa dimensiunea seturilor, iar in caz de
  egalitate, lexicografic. Evident, Reducer-ii se vor opri la o bariera inainte
  de aceasta procedura, pentru ca array-ul de liste sa fie complet populat.
    Imediat dupa sortarea unei liste, un Reducer va crea fisierul corespunzator
  acelei liste si va scrie in acesta.
