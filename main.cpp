/* 
 * File:   main.cpp
 * Author: daniel
 *
 * Created on 23 grudzień 2015, 13:50
 */

#include <cstdlib>
#include <iostream>
#include <vector>
#include <time.h>
#include <openssl/ssl.h> 
#include <ff/farm.hpp>
#include <ff/pipeline.hpp>
#include "global_fun.h"

using namespace std;
using namespace ff;

#define ulong unsigned long int

//Przygotowanie zadania, które wiemy dokładnieile będzie trwało

void czekaj(int iSekundy) {
    for (clock_t koniec = clock() + iSekundy * CLOCKS_PER_SEC; clock() < koniec;)
        continue;

}

struct WorkerA : ff_node_t<long> {

    long *svc(long *task) {
        //worker szybki,nie usypiamy 
        std::cout << "WorkerA has got the task " << *task << "\n";
        /* if  *task % 5==0 licz zadanie małe 
         * if  *task % 5==1 licz zadanie srednie
         * if  *task % 5 =2 licz zadanie duze
         * if  * task % 5 =3  licz zadanie bardzoduze
         * if  *task  % 5 =4 licz zadanie olbrzymie
       
         */
        /* rejestrujemy jakie taski dostał  ten worker w kolejnych rozdaniach */


        if (*task % 5 == 0) czekaj(60);
        if (*task % 5 == 1) czekaj(120);
        if (*task % 5 == 2) czekaj(180);
        if (*task % 5 == 3) czekaj(480);
        if (*task % 5 == 4) czekaj(600);
        //kiedys bedzie tak:
        //double exec_time = matrix_multiplication(128, 5);
        // pierwszy argument to wymiar macierzy kwadratowej, tutaj 128x128
        // drugi, to ile razy ma się to przemnożyć

        return task;
    }
};

struct WorkerB : ff_node_t<long> {

    long *svc(long *task) {
        //worker wolniejszy, usypiamy 
        czekaj(30);
        std::cout << "WorkerB has got the task " << *task << "\n";
        /* if  *task % 5==0 licz zadanie małe
         * if  *task % 5==1 licz zadanie srednie
         * if  *task % 5 =2 licz zadanie duze
         * if  * task % 5 =3  licz zadanie bardzoduze
         * if  *task  % 5 =4 licz zadanie olbrzymie
       
         */

        if (*task % 5 == 0) czekaj(60);
        if (*task % 5 == 1) czekaj(120);
        if (*task % 5 == 2) czekaj(180);
        if (*task % 5 == 3) czekaj(480);
        if (*task % 5 == 4) czekaj(600);
        //double exec_time = matrix_multiplication(128, 5);
        // pierwszy argument to wymiar macierzy kwadratowej, tutaj 128x128
        // drugi, to ile razy ma się to przemnożyć

        return task;
    }
};

struct WorkerC : ff_node_t<long> {

    long *svc(long *task) {
        //worker wolniejszy, usypiamy 
        czekaj(75);
        std::cout << "WorkerC has got the task " << *task << "\n";
        /* if  *task % 5==0 licz zadanie małe
         * if  *task % 5==1 licz zadanie srednie
         * if  *task % 5 =2 licz zadanie duze
         * if  * task % 5 =3  licz zadanie bardzoduze
         * if  *task  % 5 =4 licz zadanie olbrzymie
       
         */

        if (*task % 5 == 0) czekaj(60);
        if (*task % 5 == 1) czekaj(120);
        if (*task % 5 == 2) czekaj(180);
        if (*task % 5 == 3) czekaj(480);
        if (*task % 5 == 4) czekaj(600);
        //double exec_time = matrix_multiplication(128, 5);
        // pierwszy argument to wymiar macierzy kwadratowej, tutaj 128x128
        // drugi, to ile razy ma się to przemnożyć

        return task;
    }
};

struct WorkerD : ff_node_t<long> {
    ;

    long *svc(long *task) {
        //worker wolniejszy, usypiamy 
        czekaj(120);
        std::cout << "WorkerD has got the task " << *task << "\n";

        /* if  *task % 5==0 licz zadanie małe
         * if  *task % 5==1 licz zadanie srednie
         * if  *task % 5 =2 licz zadanie duze
         * if  * task % 5 =3  licz zadanie bardzoduze
         * if  *task  % 5 =4 licz zadanie olbrzymie
       
         */

        if (*task % 5 == 0) czekaj(60);
        if (*task % 5 == 1) czekaj(120);
        if (*task % 5 == 2) czekaj(180);
        if (*task % 5 == 3) czekaj(480);
        if (*task % 5 == 4) czekaj(600);
        //double exec_time = matrix_multiplication(128, 5);
        // pierwszy argument to wymiar macierzy kwadratowej, tutaj 128x128
        // drugi, to ile razy ma się to przemnożyć

        return task;
    }
};

struct WorkerE : ff_node_t<long> {

    long *svc(long *task) {
        //worker wolniejszy, usypiamy 
        czekaj(150);
        std::cout << "WorkerD has got the task " << *task << "\n";

        /* if  *task % 5==0 licz zadanie małe
         * if  *task % 5==1 licz zadanie srednie
         * if  *task % 5 =2 licz zadanie duze
         * if  * task % 5 =3  licz zadanie bardzoduze
         * if  *task  % 5 =4 licz zadanie olbrzymie
       
         */

        if (*task % 5 == 0) czekaj(60);
        if (*task % 5 == 1) czekaj(120);
        if (*task % 5 == 2) czekaj(180);
        if (*task % 5 == 3) czekaj(480);
        if (*task % 5 == 4) czekaj(600);
        //double exec_time = matrix_multiplication(128, 5);
        // pierwszy argument to wymiar macierzy kwadratowej, tutaj 128x128
        // drugi, to ile razy ma się to przemnożyć

        return task;
    }
};

struct Emitter : ff_node_t<long> {
    // składowe publiczne struktury
    ff_loadbalancer * const lb;
    const long size = 100; //liczba rozsylanych zadan w pojedynczym batchu 

    // metody struktury

    Emitter(ff_loadbalancer * const lb) : lb(lb) {
    } // konstruktor

    //MODEL REFERECYJNY Z ROWNYM ROZZYLEM ZADAN -NIELOSOWYM CO 5 MIN
    //pojedynczy serwis rozsyla 100 zadan 12 razy wciagu godziny:60*60 sek=3600sek, 
    //rozsyl  w modelu referencyjnym co 5min=5x60sek=300sek 

    // funkcja rozsyłająca zadania

    long *svc(long *) {

        for (clock_t koniec = clock() + 3600 * CLOCKS_PER_SEC; clock() < koniec;) {
            if (koniec % 300 == 0) {
                //rozsylanie pojedynczej paczki 
                for (long i = 0; i <= size; ++i) {
                    lb->broadcast_task(new long(i));

                    //EMITER MUSI MIEC DOSTEP DO ZMIENNEJ GLOBALNEGO CZASU 

                    /*rozrzuca taski po workerach  mamy 100 liczb do rozrzucenia
                     powiedzmy, ze na razie zrobimy tak: do workerow trafiaja liczby: 1,2... lub 100.
                     * powiedzmy ze mamy 5 typow zadan: male, srednie,duze, bardzo duze, olbrzymie
        
         
                     */
                }
            }
            continue;
        }

        return EOS;

    }
};

struct EmitterBBS : ff_node_t<long> {
    ff_loadbalancer * const lb;
    const long size = 100;

    //MODEL Z BBSOWYM ROZSYLEM ZADAN - LOSOWYM ZGODNIE Z BBS 
    //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek, ale robi to zgodnie zmodelem BBS

    EmitterBBS(ff_loadbalancer * const lb) : lb(lb) {
    }

    //EMITER WYZNACZA SOBIE MOMENTY DO ROZSYLU ZADAN, POTRZeBUJEMY TYLE ELEMENTOW CIAGU KEY, ABY POJAWILO SIE W NIM 12 JEDYNEK  

    long *svc(long *) {
        ulong *key;
        //ulong *moments;
        ulong N = 100;
        key = generateBBSKey(N);

        ulong stop = 0; // kiedy wystapila dwunasta jedynka,

        for (ulong i = 0; i < N; i++) {
            ulong *LicznikJedynek; //podaje ile jedynek wysapilo do tej w naszym ciagu - po wylosowaniu kazdego kolejnego elementu    
            int LiczbaJedynek; //podaje ile jedynek wysapilo do calym  naszym ciagu 


            // key             010011000111000 ......1
            //LicznikJedynek   011122222345555.......12
            //LiczbaJedynek 12.. szukamy kiedy wypadnie dwunasta jedynka 
            //stop=25,powiedzmy,ze jako 25ta w ciagu zer i jedynek 
            if (key[i] == 1) {
                LiczbaJedynek = LiczbaJedynek + 1;
                LicznikJedynek[i] = LiczbaJedynek;
                if (LiczbaJedynek == 12) stop = i;
            } else

                LicznikJedynek[i] = LiczbaJedynek;
        }

        //wyznaczenie ilosci przedzialow,na ktore dzielimy godzine tj 3600 sek:

        ulong TimeUnit = floor(3600 / stop); //np  floor 3600/25=  144  co 144sek  nastepuje check, czy trzeba rozsylac, czy nie 
        //ulong jest typem naturalnym,wiec jesli wyskoczy ulamek, trzeba zakoraglic,zaokraglamy w dol bo mamy ograniczenia czasowe od gory 
        //TimeUnit jest paczka czasu, w ktorej jesli wypadla  jedynka to rozsylamy zadania, jesli zero, nic nie robimy 

        //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek,

        int j = 0; //ile razy probujemy rozsylac 

        for (clock_t koniec = clock() + 3600 * CLOCKS_PER_SEC; clock() < koniec;) {

            if (koniec % TimeUnit == 0) {
                j = j + 1;

                if (key[j] == 1 && (j <= stop)) {
                    //rozsylanie pojedynczej paczki 
                    for (long i = 0; i <= size; ++i) {
                        lb->broadcast_task(new long(i));

                        /*rozrzuca taski po workerach  mamy 100 liczb do rozrzucenia
                         powiedzmy, ze na razie zrobimy tak: do workerow trafiaja liczby: 1,2... lub 100.
                         * powiedzmy ze mamy 5 typow zadan: male, srednie,duze, bardzo duze, olbrzymie
                         */
                    }
                }

            } //koniec opercji dotyczacej TimeUnit
            continue;
        }

        free(key);
        return EOS;
    }//koniec serwisu
};

struct EmitterSHA : ff_node_t<long> {
    //MODEL Z sha ROZSYLEM ZADAN -LOSOWYM ZGODNIE Z sha 
    //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek, 

    ff_loadbalancer * const lb;
    const long size = 100;

    EmitterSHA(ff_loadbalancer * const lb) : lb(lb) {
    }

    //EMITER WYZNACZA SOBIEMOMENTY DO ROZSYLU ZADAN, POTRZBUJEMY TYLE
    //ELEMENTOWCIAGU sha2, ABY POJAWILO SIE W NIM 12 JEDYNEK  
    //przygotowanie zmienych do zarzadzania czasem  rozsylu zadan 
    //SHA512

    long *svc(long *) {
        char str[] = "Seed"; // wiadomosc
        char *mdString = NULL; // skrot
        mdString = new char[SHA512_DIGEST_LENGTH * 2 + 1];
        get_SHA512(str, mdString); // przekazujemy wiadomosc i zaalokowane miejsce na skrot

        ulong stop = 0; // kiedy wystapila dwunasta jedynka,
        for (ulong i = 0; i < N; i++) {
            ulong *LicznikJedynek; //podaje ile jedynek wysapilo do tej w naszym ciagu - po wylosowaniu kazdego kolejnego elementu    
            int LiczbaJedynek; //podaje ile jedynek wysapilo do calym  naszym ciagu 


            // key             010011000111000 ......1
            //LicznikJedynek   011122222345555.......12
            //LiczbaJedynek 12.. szukamy kiedy wypadnie dwunasta jedynka 
            //stop=25,powiedzmy,ze jako 25ta w ciagu zer i jedynek 
            if (mdString[i] == 1) {
                LiczbaJedynek = LiczbaJedynek + 1;
                LicznikJedynek[i] = LiczbaJedynek;
                if (LiczbaJedynek == 12) stop = i;
            } else

                LicznikJedynek[i] = LiczbaJedynek;

        }

        //wyznaczenie ilosci przedzialow,na ktore dzielimy godzine tj 3600 sek:
        ulong TimeUnit = floor(3600 / stop); //np  floor 3600/25=  144  co144 nastepuje check, czy trzeba rozsylac, czy nie 
        //ulong jest typem naturalnym,wiec jesli wyskoczy ulamek, trzeba zakoraglic,zaokraglamy w dol bo mamy ograniczenia czasowe od gory 
        //TimeUnit jest paczka czasu, w ktorej jesli wypadla  jedynka to rozsylamy zadania, jesli zero, nic nie robimy 

        //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek,

        int j = 0; //ile razy probujemy rozsylac 

        for (clock_t koniec = clock() + 3600 * CLOCKS_PER_SEC; clock() < koniec;) {


            if (koniec % TimeUnit == 0) {
                j = j + 1;

                if (key[j] == 1 && (j <= stop)) {
                    //rozsylanie pojedynczej paczki 
                    for (long i = 0; i <= size; ++i) {
                        lb->broadcast_task(new long(i));

                        /*rozrzuca taski po workerach  mamy 100 liczb do rozrzucenia
                         powiedzmy, ze na razie zrobimy tak: do workerow trafiaja liczby: 1,2... lub 100.
                         * powiedzmy ze mamy 5 typow zadan: male, srednie,duze, bardzo duze, olbrzymie
                         */
                    }
                }

            } //koniec opercji dotyczacej TimeUnit
            continue;
        }

        delete[] mdString;
        return EOS;
    }//koniec serwisu

};

struct EmitterU : ff_node_t<long> {
    //MODEL Z ROZKLADEMJEDNOSTAJNYM  ROZSYLEM ZADAN -LOSOWYM 
    //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek, 
    ff_loadbalancer * const lb;
    const long size = 100;

    EmitterU(ff_loadbalancer * const lb) : lb(lb) {
        srand((unsigned int) time((time_t *) NULL));
    }

    //EMITER WYZNACZA SOBIE MOMENTY DO ROZSYLU ZADAN,  
    //przygotowanie zmienych do zarzadzania czasem  rozsylu zadan 

    bool czyBylaWylosowana(int iLiczba, int tab[], int ile) {
        if (ile <= 0)
            return false;

        int i = 0;
        do {
            if (tab[ i ] == iLiczba)
                return true;

            i++;
        } while (i < ile);

        return false;
    }

    int wylosuj() {
        return ( rand() % 60) + 1;
    }

    long *svc(long *) {
        // jednostajny  POTRZEBUJEMY 12 LICZB z zakresu 1 do 60, bez powtorzen  !!
        //tablica wylosowanych przechowuje liczby 

        int wylosowane[ 12 ];
        int wylosowanych = 0;
        do {
            int liczba = wylosuj();
            if (czyBylaWylosowana(liczba, wylosowane, wylosowanych) == false) {
                wylosowane[ wylosowanych ] = liczba;
                wylosowanych++;
            } //if
        } while (wylosowanych < 5);

        wylosowanych = 0;
        do {
            std::cout << wylosowane[ wylosowanych ] << std::endl;
            wylosowanych++;
        } while (wylosowanych < 12);

        //PO TEJ PROCEDURZE MAMY 12 LICZB Z ZAKRESU 1..60  

        //CZYLI ROZSYL NASTEPUJE W WYLOSOWANE[1]*60 ,WYLOSOWANE[2]*60,....WYLOSOWANE[12]*60  SEK 

        //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek,

        for (clock_t koniec = clock() + 3600 * CLOCKS_PER_SEC; clock() < koniec;) {

            if (koniec % (60 * wylosowane[1]) == 0 || 
                    koniec % (60 * wylosowane[2]) == 0 || 
                    koniec % (60 * wylosowane[3]) == 0 || 
                    koniec % (60 * wylosowane[4]) == 0 || 
                    koniec % (60 * wylosowane[5]) == 0 || 
                    koniec % (60 * wylosowane[6]) == 0 || 
                    koniec % (60 * wylosowane[7]) == 0 || 
                    koniec % (60 * wylosowane[8]) == 0 || 
                    koniec % (60 * wylosowane[9]) == 0 || 
                    koniec % (60 * wylosowane[10]) == 0 || 
                    koniec % (60 * wylosowane[11]) == 0 || 
                    koniec % (60 * wylosowane[12]) == 0 ){

                //rozsylanie pojedynczej paczki 
                for (long i = 0; i <= size; ++i) {
                    lb->broadcast_task(new long(i));

                    /*rozrzuca taski po workerach  mamy 100 liczb do rozrzucenia
                     powiedzmy, ze na razie zrobimy tak: do workerow trafiaja liczby: 1,2... lub 100.
                     * powiedzmy ze mamy 5 typow zadan: male, srednie,duze, bardzo duze, olbrzymie
                     */
                }
            }
            continue;
        }
        return EOS;
    }//koniec serwisu



};

//Poisson  TODO

struct Collector : ff_node_t<long> {

    Collector(ff_gatherer * const gt) : gt(gt) {
    }
    ff_gatherer * const gt;

    long *svc(long *task) {
        std::cout << "received task from Worker " << gt->get_channel_id() << "\n";
        if (gt->get_channel_id() == 0) delete task;
        return GO_ON;
    }
};


int main(int argc, char **argv) {


    // Mnozenie macierzy
    double exec_time = 0.0;
    // macierz 128x128 piec razy liczona
    // nasze zadanie testowe - niezrownoleglone
    exec_time = matrix_multiplication(128, 5);
    printf("Execution time: %lf ms\n", exec_time);


    //You can pass in a pointer to a time_t object that time will fill 
    //up with the current time (and the return value is the same one that you pointed to).
    //If you pass in NULL, it just ignores it and merely returns a new time_t object that
    // represents the current time.

    // - test pomiaru czasu
    printf("Rozpoczynam odliczanie.\n");

    time_t czasStart = time(NULL);
    czekaj(3);
    time_t czasStop = time(NULL);

    printf("Uplynelo %.2fsek.", difftime(czasStop, czasStart));


    // - test pomiaru czasu 2
    time_t czas;
    time(& czas);
    printf("Czas lokalny: %s\n", ctime(& czas));

    //

    //ilosc workerow danego typu,tutaj -po jednym 
    int nworkers = 1;

    std::vector<std::unique_ptr<ff_node> > Workers;


    for (int i = 1; i <= nworkers; ++i) {
        Workers.push_back(make_unique<WorkerA>());
        printf("POWSTAL WORKER TYPU  A \n");
    }
    for (int i = 1; i <= nworkers; ++i) {
        Workers.push_back(make_unique<WorkerB>());
        printf("POWSTAL WORKER TYPU  B \n");
    }
    for (int i = 1; i <= nworkers; ++i) {
        Workers.push_back(make_unique<WorkerC>());
        printf("POWSTAL WORKER TYPU  C \n");
    }

    for (int i = 1; i <= nworkers; ++i) {
        Workers.push_back(make_unique<WorkerD>());
        printf("POWSTAL WORKER TYPU  D \n");
    }

    for (int i = 1; i <= nworkers; ++i) {
        Workers.push_back(make_unique<WorkerE>());
        printf("POWSTAL WORKER TYPU  E \n");
    }


    //FARMA Z EMITEREM REFERENCYJNYM 
    ff_Farm<> farm(std::move(Workers));
    Emitter E(farm.getlb());
    Collector C(farm.getgt());
    farm.add_emitter(E); // add the specialized emitter
    farm.add_collector(C);
    if (farm.run_and_wait_end() < 0) error("running  REFERENCE farm");

    //FARMA Z EMITEREM BBS 
    ff_Farm<> farmBBS(std::move(Workers));
    EmitterBBS E(farmBBS.getlb());
    Collector C(farmBBS.getgt());
    farmBBS.add_emitter(E); // add the specialized emitter
    farmBBS.add_collector(C);
    if (farmBBS.run_and_wait_end() < 0) error("running BBS  farm");

    //FARMA Z EMITEREM SHA 
    ff_Farm<> farmSHA(std::move(Workers));
    EmitterSHA E(farmSHA.getlb());
    Collector C(farmSHA.getgt());
    farmSHA.add_emitter(E); // add the specialized emitter
    farmSHA.add_collector(C);
    if (farmSHA.run_and_wait_end() < 0) error("running SHA farm");

    //FARMA Z EMITEREM JEDNOSTAJNYM 
    ff_Farm<> farmU(std::move(Workers));
    EmitterU E(farmU.getlb());
    Collector C(farmU.getgt());
    farmU.add_emitter(E); // add the specialized emitter
    farmU.add_collector(C);
    if (farmU.run_and_wait_end() < 0) error("running UNITED farm");

    cout << "Press enter to continue ..." << endl;
    cin.get();

    return 0;
}

/*
int main(int argc, char **argv) {

    int nworkers = 4;

    std::vector<std::unique_ptr<ff_node> > Workers;


    for (int i = 0; i < nworkers / 2; ++i) Workers.push_back(make_unique<WorkerA>());
    for (int i = nworkers / 2; i < nworkers; ++i) Workers.push_back(make_unique<WorkerB>());

    ff_Farm<> farm(std::move(Workers));
    Emitter E(farm.getlb());
    Collector C(farm.getgt());
    farm.add_emitter(E); // add the specialized emitter
    farm.add_collector(C);
    if (farm.run_and_wait_end() < 0) error("running farm");

    cout << "Press enter to continue ..." << endl;
    cin.get();

    // Mnożenie macierzy
    double exec_time = 0.0;
    // macierz 128x128 pięć razy liczona
    exec_time = matrix_multiplication(128, 5);
    printf("Execution time: %lf ms\n", exec_time);

    //SHA512
    try {
        char str[] = "Seed"; // wiadomosc
        char *mdString = NULL; // skrot
        mdString = new char[SHA512_DIGEST_LENGTH * 2 + 1];
        get_SHA512(str, mdString); // przekazujemy wiadomosc i zaalokowane miejsce na skrot
        printf("SHA512 digest: %s\n", mdString);
        delete[] mdString;
    } catch (bad_alloc) {
        cerr << "Blad alokacji (SHA)" << endl;
    }
    cout << endl;

    //BBS
    ulong *key;
    ulong N;
    N = 100;
    key = generateBBSKey(N);
    for (ulong i = 0; i < N; i++) {
        std::cout << key[i];
    }
    free(key);


    cout << endl;

    return 0;
}
*/