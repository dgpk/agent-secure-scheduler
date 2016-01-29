/* 
 * File:   main.cpp
 * Author: daniel
 *
 * Created on 23 grudzień 2015, 13:50
 */
//#define _GLIBCXX_USE_NANOSLEEP
//#include <thread>
//#include <string>

#include <cstdlib>
#include <iostream>
#include <vector>
#include <time.h>
#include <openssl/ssl.h> 
#include <ff/farm.hpp>
#include <ff/pipeline.hpp>
#include "global_fun.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>

using namespace std;
using namespace ff;

#define REF
#define BBS
#define SHA
#define U

#define ulong unsigned long int
#define numOfPackages 12
#define sizeOfPackage 100

sem_t semTable[numOfPackages];
//int taskTable[numOfPackages];
//pthread_t *threads;

//struct TimerData {
//    int packageID = 0;
//    int time = 0;
//};

//Przygotowanie zadania, które wiemy dokładnieile będzie trwało

void czekaj(int iSekundy) {
    for (clock_t koniec = clock() + iSekundy * CLOCKS_PER_SEC; clock() < koniec;)
        continue;

}

// Inicjalizujemy semafory (funkcja wywoływana raz w main)

void init_sem() {
    for (int i = 0; i < numOfPackages; i++) {
        sem_init(&semTable[i], 0, 1);
        //taskTable[i] = sizeOfPackage;
    }
    //threads = new pthread_t[numOfPackages];
}

// Opuszczamy semafory (funkcja wywoływana w każdym emmiterze na początku)

void down_sem() {
    for (int i = 0; i < numOfPackages; i++) {
        sem_wait(&semTable[i]);
        //taskTable[i] = sizeOfPackage;
    }
}

// Niszczymy semafory (funkcja wywoływana raz w main - na końcu)

void destroy() {
    for (int i = 0; i < numOfPackages; i++)
        sem_destroy(&semTable[i]);
    //delete[] threads;
}

// Funkcja która zarządza dostępem do paczek zadań 
// czyli zadania przychodzą w określonych odstępach czasu
// Wyświetla informacje o paczce i ile musi czekac na przyjscie
// Po tym czasie podnosi semafor, czyli daje dostep do tych danych
// Po podniesieniu wszystkich semaforów wątek ginie

void* thread_timer(void *ptr) {
    int *timeTable = (int *) ptr;
    for (int i = 0; i < numOfPackages; i++) {
        cout << "Paczka " << i << "  Watek czeka " << timeTable[i] << endl;
        sleep(timeTable[i]);
        sem_post(&semTable[i]);
        cout << "podniesiono semafor " << i << endl;
    }
    pthread_exit(0);

}

struct WorkerA : ff_node_t<long> {

    long *svc(long *task) override {
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

    long *svc(long *task) override {
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

    long *svc(long *task) override {
        //worker wolniejszy, usypiamy 
        czekaj(75);
        //std::cout << "WorkerC has got the task " << *task << "\n";
        printf("WorkerC has got the task %ld\n", *((long*) task));
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

    long *svc(long *task) override {
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

    long *svc(long *task) override {
        //worker wolniejszy, usypiamy 
        czekaj(150);
        std::cout << "WorkerE has got the task " << *task << "\n";

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
    unsigned int wait_time = 300; // po rozesłaniu paczki (uwaga! nie po wykonaniu wszystkich zadań) czekamy tyle sekund (tut. 5 minut)

    Emitter() {
        srand((unsigned int) time((time_t *) NULL)); // korzystamy z funkcji rand  // ZA KAŻDYM RAZEM WYLOSUJEMY INNY ROZKŁAD  !!!!!!!!!!!!!!
    }
    //MODEL REFERECYJNY Z ROWNYM ROZZYLEM ZADAN -NIELOSOWYM CO 5 MIN
    //pojedynczy serwis rozsyla 100 zadan 12 razy wciagu godziny:60*60 sek=3600sek, 
    //rozsyl  w modelu referencyjnym co 5min=5x60sek=300sek 
    // funkcja rozsyłająca zadania

    long *svc(long *) override {
        down_sem();
        //model ma rozesłać paczki 12 razy, więc...
        for (int n = 0; n < numOfPackages; n++) {
            cout << "Paczka nr " << n << endl;
            //Rozsyłamy zadania
            for (long i = 0; i < sizeOfPackage; ++i) {
                ff_send_out(new long(i));
            }
            cout << "Paczka nr " << n << " - wszystkie zadania zostaly rozeslane" << endl;
            cout << "Czekamy 5 minut - gromadzimy zadania :)" << endl;
            //czekamy 5 minut (300 sec)
            sleep(wait_time);
        }
        return EOS;
    }
};

struct EmitterBBS : ff_node_t<long> {

    EmitterBBS() {
        srand((unsigned int) time((time_t *) NULL)); // korzystamy z funkcji rand  // ZA KAŻDYM RAZEM WYLOSUJEMY INNY ROZKŁAD 
    }
    //MODEL Z BBSOWYM ROZSYLEM ZADAN - LOSOWYM ZGODNIE Z BBS 
    //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek, ale robi to zgodnie zmodelem BBS


    //EMITER WYZNACZA SOBIE MOMENTY DO ROZSYLU ZADAN, POTRZeBUJEMY TYLE ELEMENTOW CIAGU KEY, ABY POJAWILO SIE W NIM 12 JEDYNEK  

    long *svc(long *) override {
        down_sem();
        ulong *key;
        //ulong *moments;
        ulong N = 100;
        key = generateBBSKey(N); //  ZA KAŻDYM RAZEM DOSTANIEMY INNY ROZKŁAD (chyba)
        ulong *LicznikJedynek; //podaje ile jedynek wysapilo do tej w naszym ciagu - po wylosowaniu kazdego kolejnego elementu 
        LicznikJedynek = new ulong[N];
        int liczbaJedynek = 1; //podaje ile jedynek wysapilo do calym  naszym ciagu
        //int start = 0; // kiedy wystapila pierwsza jedynka
        int stop = 0; // kiedy wystapila dwunasta jedynka,
        pthread_t time_thread;

        //TimerData timerData[numOfPackages];
        int *sleepTable;
        sleepTable = new int[numOfPackages];
        for (int i = 0; i < numOfPackages; i++) {
            (sleepTable[i]) = 0;
        }
        //memset(sleepTable, 0, sizeof (sleepTable));

        for (int i = 0; i < N; i++) {
            // key             010011000111000 ......1
            //LicznikJedynek   011122222345555.......12
            //LiczbaJedynek 12.. szukamy kiedy wypadnie dwunasta jedynka 
            //stop=25,powiedzmy,ze jako 25ta w ciagu zer i jedynek 

            //????
            (sleepTable[liczbaJedynek])++;
            if (key[i] == 1) {

                //LiczbaJedynek++;
                //LicznikJedynek[i] = LiczbaJedynek;
                //if (LiczbaJedynek == 12) stop = i;
                liczbaJedynek++;
                if (liczbaJedynek == 12) {
                    stop = i;
                    break;
                }
            }

        }

        cout << "KLUCZ:  ";
        for (int i = 0; i < N; i++) {
            std::cout << key[i];
        }
        cout << "\n\ntablica sleep:  ";
        for (int i = 0; i < numOfPackages; i++) {
            std::cout << (sleepTable[i]);
        }
        cout << endl;
        if (liczbaJedynek != numOfPackages) {
            cerr << "Liczba jedynek nie jest rowna " << numOfPackages << endl;
            free(key);
            delete[] LicznikJedynek;
            getchar();
            exit(-1);
        }

        //wyznaczenie ilosci przedzialow,na ktore dzielimy godzine tj 3600 sek:

        ulong TimeUnit = floor(3600 / (stop + 1)); //np  floor 3600/25=  144  co 144sek  nastepuje check, czy trzeba rozsylac, czy nie 
        //ulong jest typem naturalnym,wiec jesli wyskoczy ulamek, trzeba zakoraglic,zaokraglamy w dol bo mamy ograniczenia czasowe od gory 
        //TimeUnit jest paczka czasu, w ktorej jesli wypadla  jedynka to rozsylamy zadania, jesli zero, nic nie robimy 

        //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek,

        // UWAGA! OSTATNI SCHEDUL BĘDZIE WYKONANY TimeUnit przed osiągnięciem godziny (w momencie 3600 - TimeUnit)
        // CZY TAK TO MA BYĆ???????
        // Czy nie powinien wpierw zaczac harmonogramowac, a potem dopiero czekac, czyli to samo ale dla 11 zadan?
        cout << "TimeUnit: " << TimeUnit << endl;
        for (int n = 0; n < numOfPackages; n++) {
            (sleepTable[n]) *= TimeUnit;

        }
        pthread_create(&(time_thread), NULL, thread_timer, (void*) sleepTable);

        for (int n = 0; n < numOfPackages; n++) {
            cout << "Paczka nr " << n << "  czeka na semafor" << endl;
            sem_wait(&semTable[n]);
            cout << "Paczka nr " << n << " rozsylana" << endl;
            //Rozsyłamy zadania
            for (long i = 0; i < sizeOfPackage; ++i) {
                ff_send_out(new long(i));
            }
            cout << "Paczka nr " << n << " - wszystkie zadania zostaly rozeslane" << endl;
            sem_post(&semTable[n]);
        }
        pthread_join(time_thread, NULL);

        free(key);
        delete[] LicznikJedynek;
        return EOS;
    }//koniec serwisu
};

struct EmitterSHA : ff_node_t<long> {

    EmitterSHA() {
        srand((unsigned int) time((time_t *) NULL)); // korzystamy z funkcji rand  // ZA KAŻDYM RAZEM WYLOSUJEMY INNY ROZKŁAD 
    }

    // Funkcja zamieniajaca hex to bin

    const char* hex_char_to_bin(char c) {
        switch (toupper(c)) {
            case '0': return "0000";
                break;
            case '1': return "0001";
                break;
            case '2': return "0010";
                break;
            case '3': return "0011";
                break;
            case '4': return "0100";
                break;
            case '5': return "0101";
                break;
            case '6': return "0110";
                break;
            case '7': return "0111";
                break;
            case '8': return "1000";
                break;
            case '9': return "1001";
                break;
            case 'A': case 'a': return "1010";
                break;
            case 'B': case 'b': return "1011";
                break;
            case 'C': case 'c': return "1100";
                break;
            case 'D': case 'd': return "1101";
                break;
            case 'E': case 'e': return "1110";
                break;
            case 'F': case 'f': return "1111";
                break;
            default:
                cerr << "Error hex to bin" << endl;
                getchar();
                exit(-1);
                break;
        }
    }

    //MODEL Z sha ROZSYLEM ZADAN -LOSOWYM ZGODNIE Z sha 
    //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek, 


    //EMITER WYZNACZA SOBIEMOMENTY DO ROZSYLU ZADAN, POTRZBUJEMY TYLE
    //ELEMENTOWCIAGU sha2, ABY POJAWILO SIE W NIM 12 JEDYNEK  
    //przygotowanie zmienych do zarzadzania czasem  rozsylu zadan 
    //SHA512

    long *svc(long *) override {
        down_sem();
        int N = 512; // liczba bitow
        int str_len = rand() % 10;
        char *str = new char[str_len]; // wiadomosc
        gen_random(str, str_len); //ZAWSZE DOSTANIEMY INNY ROZKŁAD
        char *mdString = NULL; // skrot
        char SHAbin[N + 1]; // +1 bo jeszcze znak konca tekstu
        mdString = new char[SHA512_DIGEST_LENGTH * 2 + 1];
        get_SHA512(str, mdString); // przekazujemy wiadomosc i zaalokowane miejsce na skrot
        for (int i = 0; i < SHA512_DIGEST_LENGTH * 2; i++) {

            strncpy(&(SHAbin[i * 4]), hex_char_to_bin(mdString[i]), 4);
        }
        SHAbin[N] = '\0'; // znak konca
        //printf("%s\n\n", SHAbin);
        ulong *LicznikJedynek; //podaje ile jedynek wysapilo do tej w naszym ciagu - po wylosowaniu kazdego kolejnego elementu 
        LicznikJedynek = new ulong[N];
        int liczbaJedynek = 1; //podaje ile jedynek wysapilo do calym  naszym ciagu
        //int start = 0; // kiedy wystapila pierwsza jedynka
        int stop = 0; // kiedy wystapila dwunasta jedynka,
        pthread_t thread;

        //TimerData timerData[numOfPackages];
        int *sleepTable; // ta tablica informuje ile czekac na kolejne wysłanie zadań
        sleepTable = new int[numOfPackages];
        for (int i = 0; i < numOfPackages; i++) {
            (sleepTable[i]) = 0;
        }
        //memset(sleepTable, 0, sizeof (sleepTable));

        for (int i = 0; i < N; i++) {
            // key             010011000111000 ......1
            //LicznikJedynek   011122222345555.......12
            //LiczbaJedynek 12.. szukamy kiedy wypadnie dwunasta jedynka 
            //stop=25,powiedzmy,ze jako 25ta w ciagu zer i jedynek 

            (sleepTable[liczbaJedynek])++;
            if (SHAbin[i] == '1') {

                liczbaJedynek++;
                if (liczbaJedynek == 12) {
                    stop = i;
                    break;
                }
            }

        }

        cout << "KLUCZ:  ";
        for (int i = 0; i < N; i++) {
            std::cout << SHAbin[i];
        }
        cout << "\n\ntablica sleep:  ";
        for (int i = 0; i < numOfPackages; i++) {
            std::cout << (sleepTable[i]);
        }
        cout << endl;
        if (liczbaJedynek != numOfPackages) {
            cerr << "Liczba jedynek nie jest rowna " << numOfPackages << endl;
            getchar();
            delete[] LicznikJedynek;
            exit(-1);
        }

        //wyznaczenie ilosci przedzialow,na ktore dzielimy godzine tj 3600 sek:

        ulong TimeUnit = floor(3600 / (stop + 1)); //np  floor 3600/25=  144  co 144sek  nastepuje check, czy trzeba rozsylac, czy nie 
        //ulong jest typem naturalnym,wiec jesli wyskoczy ulamek, trzeba zakoraglic,zaokraglamy w dol bo mamy ograniczenia czasowe od gory 
        //TimeUnit jest paczka czasu, w ktorej jesli wypadla  jedynka to rozsylamy zadania, jesli zero, nic nie robimy 

        //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek,

        // UWAGA! OSTATNI SCHEDUL BĘDZIE WYKONANY TimeUnit przed osiągnięciem godziny (w momencie 3600 - TimeUnit)
        // CZY TAK TO MA BYĆ???????
        // Czy nie powinien wpierw zaczac harmonogramowac, a potem dopiero czekac, czyli to samo ale dla 11 zadan?
        cout << "TimeUnit: " << TimeUnit << endl;
        for (int n = 0; n < numOfPackages; n++) {
            (sleepTable[n]) *= TimeUnit;

        }
        pthread_create(&(thread), NULL, thread_timer, (void*) sleepTable);

        for (int n = 0; n < numOfPackages; n++) {
            cout << "Paczka nr " << n << "  czeka na semafor" << endl;
            sem_wait(&semTable[n]);
            cout << "Paczka nr " << n << " rozsylana" << endl;
            //Rozsyłamy zadania
            for (long i = 0; i < sizeOfPackage; ++i) {
                ff_send_out(new long(i));
            }
            cout << "Paczka nr " << n << " - wszystkie zadania zostaly rozeslane" << endl;
            sem_post(&semTable[n]);
        }
        pthread_join(thread, NULL);

        delete[] LicznikJedynek;
        delete[] mdString;
        return EOS;
    }//koniec serwisu

};

struct EmitterU : ff_node_t<long> {
    //MODEL Z ROZKLADEMJEDNOSTAJNYM  ROZSYLEM ZADAN -LOSOWYM 
    //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek, 

    EmitterU() {
        srand((unsigned int) time((time_t *) NULL)); // korzystamy z funkcji rand  // ZA KAŻDYM RAZEM WYLOSUJEMY INNY ROZKŁAD  !!!!!!!!!!!!!!
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
        return ( rand() % 60) + 1; // CZY NIE POWINNO BYC 59 ?????
    }

    long *svc(long *) override {
        down_sem();
        // jednostajny  POTRZEBUJEMY 12 LICZB z zakresu 1 do 60, bez powtorzen  !!
        //tablica wylosowanych przechowuje liczby 
        pthread_t time_thread;

        int *sleepTable;
        sleepTable = new int[numOfPackages];
        sleepTable[0] = 0; // Zakladamy, że pierwsze wywołanie bez opoznienia
        int wylosowanych = 1;
        do {
            int liczba = wylosuj()*60; // w sekundach
            if (czyBylaWylosowana(liczba, sleepTable, wylosowanych) == false) {
                sleepTable[ wylosowanych ] = liczba;
                wylosowanych++;
            } //if
        } while (wylosowanych < numOfPackages);
        sort(sleepTable, sleepTable + numOfPackages);

        wylosowanych = 0;
        do {
            std::cout << sleepTable[ wylosowanych ] << std::endl;
            wylosowanych++;
        } while (wylosowanych < numOfPackages);

        //PO TEJ PROCEDURZE MAMY 12 LICZB Z ZAKRESU 1..60  

        //CZYLI ROZSYL NASTEPUJE W WYLOSOWANE[1]*60 ,WYLOSOWANE[2]*60,....WYLOSOWANE[12]*60  SEK 

        //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek,
        pthread_create(&(time_thread), NULL, thread_timer, (void*) sleepTable);

        for (int n = 0; n < numOfPackages; n++) {
            cout << "Paczka nr " << n << "  czeka na semafor" << endl;
            sem_wait(&semTable[n]);
            cout << "Paczka nr " << n << " rozsylana" << endl;
            //Rozsyłamy zadania
            for (long i = 0; i < sizeOfPackage; ++i) {
                ff_send_out(new long(i));
            }
            cout << "Paczka nr " << n << " - wszystkie zadania zostaly rozeslane" << endl;
            sem_post(&semTable[n]);
        }
        pthread_join(time_thread, NULL);

        delete[] sleepTable;
        return EOS;
    }//koniec serwisu



};

//Poisson  TODO

struct Collector : ff_node_t<long> {
    ff_gatherer * const gt;

    Collector(ff_gatherer * const gt) : gt(gt) {
    }

    long *svc(long *task) override {
        //std::cout << "received task from Worker " << gt->get_channel_id() << "\n";
        delete task;
        return GO_ON;
    }
};

/*
// C++ asynchroniczny
void callback(const std::string& data)
{
    std::cout << "Callback called because: " << data << '\n';
}
void task(int time)
{
    std::this_thread::sleep_for(std::chrono::seconds(time));
    callback("async task done");
}
int main()
{
    std::thread bt(task, 1);
    std::cout << "async task launched\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "main done\n";
    bt.join();
}
 */

void create_workers(std::vector<std::unique_ptr<ff_node> > *Workers, int nworkers) {
    for (int i = 1; i <= nworkers; ++i) {
        Workers->push_back(make_unique<WorkerA>());
        cout << "WorkerA created" << endl;
    }
    for (int i = 1; i <= nworkers; ++i) {
        Workers->push_back(make_unique<WorkerB>());
        cout << "WorkerB created" << endl;
    }
    for (int i = 1; i <= nworkers; ++i) {
        Workers->push_back(make_unique<WorkerC>());
        cout << "WorkerC created" << endl;
    }

    for (int i = 1; i <= nworkers; ++i) {
        Workers->push_back(make_unique<WorkerD>());
        cout << "WorkerD created" << endl;
    }

    for (int i = 1; i <= nworkers; ++i) {
        Workers->push_back(make_unique<WorkerE>());
        cout << "WorkerE created" << endl;
    }
}

int main(int argc, char **argv) {


    // Mnozenie macierzy
    //double exec_time = 0.0;
    // macierz 128x128 piec razy liczona
    // nasze zadanie testowe - niezrownoleglone
    //exec_time = matrix_multiplication(128, 5);
    //printf("Execution time: %lf ms\n", exec_time);


    //You can pass in a pointer to a time_t object that time will fill 
    //up with the current time (and the return value is the same one that you pointed to).
    //If you pass in NULL, it just ignores it and merely returns a new time_t object that
    // represents the current time.

    // - test pomiaru czasu
    //    printf("Rozpoczynam odliczanie.\n");
    //
    //    time_t czasStart = time(NULL);
    //    czekaj(3);
    //    time_t czasStop = time(NULL);
    //
    //    printf("Uplynelo %.2fsek.", difftime(czasStop, czasStart));
    //
    //
    //    // - test pomiaru czasu 2
    //    time_t czas;
    //    time(& czas);
    //    printf("Czas lokalny: %s\n", ctime(& czas));

    //

    //ilosc workerow danego typu,tutaj -po jednym 
    int nworkers = 1;

    std::vector<std::unique_ptr<ff_node> > Workers;

    init_sem();

    cout << "Czas start" << endl;
    clock_t begin = clock();

#ifdef REF
    //FARMA Z EMITEREM REFERENCYJNYM
    create_workers(&Workers, nworkers);
    ff_Farm<long> farm(std::move(Workers));
    farm.set_scheduling_ondemand(); // set auto scheduling
    Emitter E;
    Collector C(farm.getgt());
    farm.add_emitter(E); // add the specialized emitter
    farm.add_collector(C);

    if (farm.run_and_wait_end() < 0) error("running  REFERENCE farm");
#endif

#ifdef BBS
    //FARMA Z EMITEREM BBS
    create_workers(&Workers, nworkers);
    ff_Farm<long> farmBBS(std::move(Workers));
    farmBBS.set_scheduling_ondemand(); // set auto scheduling
    EmitterBBS EBBS;
    Collector CBBS(farmBBS.getgt());
    farmBBS.add_emitter(EBBS); // add the specialized emitter
    farmBBS.add_collector(CBBS);
    if (farmBBS.run_and_wait_end() < 0) error("running BBS  farm");
#endif

#ifdef SHA
    //FARMA Z EMITEREM SHA
    create_workers(&Workers, nworkers);
    ff_Farm<long> farmSHA(std::move(Workers));
    farmSHA.set_scheduling_ondemand(); // set auto scheduling
    EmitterSHA ESHA;
    Collector CSHA(farmSHA.getgt());
    farmSHA.add_emitter(ESHA); // add the specialized emitter
    farmSHA.add_collector(CSHA);
    if (farmSHA.run_and_wait_end() < 0) error("running SHA farm");
#endif

#ifdef U
    //FARMA Z EMITEREM JEDNOSTAJNYM
    create_workers(&Workers, nworkers);
    ff_Farm<long> farmU(std::move(Workers));
    farmU.set_scheduling_ondemand(); // set auto scheduling
    EmitterU EU;
    Collector CU(farmU.getgt());
    farmU.add_emitter(EU); // add the specialized emitter
    farmU.add_collector(CU);
    if (farmU.run_and_wait_end() < 0) error("running UNITED farm");
#endif

    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    cout << "Czas stop: " << elapsed_secs << " [s]" << endl;
    cout << "Press enter to continue ..." << endl;
    cin.get();
    destroy();
    return 0;
}