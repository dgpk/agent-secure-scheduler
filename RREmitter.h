/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RREmitter.h
 * Author: daniel
 *
 * Created on 13 lipca 2016, 12:55
 */

#ifndef RREMITTER_H
#define RREMITTER_H

#include <cstdlib>
#include <iostream>
#include <vector>
#include <time.h>
#include <openssl/ssl.h> 
#include <ff/farm.hpp>
#include <ff/pipeline.hpp>
#include "global_fun.h"
#include "GASched.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <fstream>

using namespace std;
using namespace ff;

#define REF
#define BBS
#define SHA
#define U

#define ulong unsigned long int

struct Emitter : ff_node_t<Task> {
    // składowe publiczne struktury
    //unsigned int wait_time = 300; // po rozesłaniu paczki (uwaga! nie po wykonaniu wszystkich zadań) czekamy tyle sekund (tut. 5 minut)
private:
    unsigned int processingTime, numOfPackages, sizeOfPackage;
public:

    Emitter(unsigned int _processingTime = 3600, unsigned int _numOfPackages = 12, unsigned int _sizeOfPackage = 10, unsigned int _readyTime = 300)
    : processingTime(_processingTime), numOfPackages(_numOfPackages), sizeOfPackage(_sizeOfPackage) {
        srand((unsigned int) time((time_t *) NULL)); // korzystamy z funkcji rand  // ZA KAŻDYM RAZEM WYLOSUJEMY INNY ROZKŁAD  !!!!!!!!!!!!!!
    }
    //MODEL REFERECYJNY Z ROWNYM ROZZYLEM ZADAN -NIELOSOWYM CO 5 MIN
    //pojedynczy serwis rozsyla 100 zadan 12 razy wciagu godziny:60*60 sek=3600sek, 
    //rozsyl  w modelu referencyjnym co 5min=5x60sek=300sek 
    // funkcja rozsyłająca zadania

    Task *svc(Task *) override {
        cout << "Start Emitter\n";
        //down_sem(numOfPackages);
        //model ma rozesłać paczki 12 razy, więc...
        pthread_t time_thread;
        int *sleepTable;
        sleepTable = new int[numOfPackages];
        sleepTable[0] = 0;
        for (int i = 1; i < numOfPackages; i++) {
            sleepTable[i] = processingTime / numOfPackages;
        }

        TimerData td;
        td.numOfPackages = numOfPackages;
        td.timeTable = sleepTable;
        pthread_create(&(time_thread), NULL, thread_timer, (void*) &td);

        log_batch->beginTime = clock();
        for (int n = 0; n < numOfPackages; n++) {
            cout << "Paczka nr " << n << "  czeka na semafor" << endl;
            //Rozsyłamy zadania
            log_packages[n].beginTime = clock();
            sem_wait(&semTable[n]);
            cout << "Paczka nr " << n << " rozsylana" << endl;
            for (int i = 0; i < sizeOfPackage; ++i) {
                ff_send_out(new Task(i, n, modelREF));
            }
            cout << "Paczka nr " << n << " - wszystkie zadania zostaly rozeslane" << endl;
            sem_post(&semTable[n]);
        }
        pthread_join(time_thread, NULL);
        //log_batch->endTime = clock();
        return EOS;
    }
};

struct EmitterBBS : ff_node_t<Task> {
private:
    unsigned int processingTime, numOfPackages, sizeOfPackage;
public:

    EmitterBBS(unsigned int _processingTime = 3600, unsigned int _numOfPackages = 12, unsigned int _sizeOfPackage = 10)
    : processingTime(_processingTime), numOfPackages(_numOfPackages), sizeOfPackage(_sizeOfPackage) {
        srand((unsigned int) time((time_t *) NULL)); // korzystamy z funkcji rand  // ZA KAŻDYM RAZEM WYLOSUJEMY INNY ROZKŁAD  !!!!!!!!!!!!!!
    }
    //MODEL Z BBSOWYM ROZSYLEM ZADAN - LOSOWYM ZGODNIE Z BBS 
    //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek, ale robi to zgodnie zmodelem BBS


    //EMITER WYZNACZA SOBIE MOMENTY DO ROZSYLU ZADAN, POTRZeBUJEMY TYLE ELEMENTOW CIAGU KEY, ABY POJAWILO SIE W NIM 12 JEDYNEK  

    Task *svc(Task *) override {
        cout << "Start EmitterBBS\n";
        //down_sem(numOfPackages);
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
                if (liczbaJedynek == numOfPackages) {
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

        ulong TimeUnit = floor(processingTime / (stop + 1)); //np  floor 3600/25=  144  co 144sek  nastepuje check, czy trzeba rozsylac, czy nie 
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
        TimerData td;
        td.numOfPackages = numOfPackages;
        td.timeTable = sleepTable;
        pthread_create(&(time_thread), NULL, thread_timer, (void*) &td);
        log_batch->beginTime = clock();
        for (int n = 0; n < numOfPackages; n++) {
            cout << "Paczka nr " << n << "  czeka na semafor" << endl;
            log_packages[n].beginTime = clock(); // Uwaga! doliczamy czas oczekiwania na zadania!!!!!!!
            sem_wait(&semTable[n]);
            cout << "Paczka nr " << n << " rozsylana" << endl;
            //Rozsyłamy zadania
            for (int i = 0; i < sizeOfPackage; ++i) {
                ff_send_out(new Task(i, n, modelBBS));
            }
            cout << "Paczka nr " << n << " - wszystkie zadania zostaly rozeslane" << endl;
            sem_post(&semTable[n]);
        }
        //log_batch->endTime = clock();
        pthread_join(time_thread, NULL);

        free(key);
        delete[] LicznikJedynek;
        return EOS;
    }//koniec serwisu
};

struct EmitterSHA : ff_node_t<Task> {
private:
    unsigned int processingTime, numOfPackages, sizeOfPackage;
public:

    EmitterSHA(unsigned int _processingTime = 3600, unsigned int _numOfPackages = 12, unsigned int _sizeOfPackage = 10)
    : processingTime(_processingTime), numOfPackages(_numOfPackages), sizeOfPackage(_sizeOfPackage) {
        srand((unsigned int) time((time_t *) NULL)); // korzystamy z funkcji rand  // ZA KAŻDYM RAZEM WYLOSUJEMY INNY ROZKŁAD  !!!!!!!!!!!!!!
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

    Task *svc(Task *) override {
        cout << "Start EmitterSHA\n";
        //down_sem(numOfPackages);
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
        pthread_t time_thread;

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
                if (liczbaJedynek == numOfPackages) {
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

        ulong TimeUnit = floor(processingTime / (stop + 1)); //np  floor 3600/25=  144  co 144sek  nastepuje check, czy trzeba rozsylac, czy nie 
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
        TimerData td;
        td.numOfPackages = numOfPackages;
        td.timeTable = sleepTable;
        pthread_create(&(time_thread), NULL, thread_timer, (void*) &td);
        log_batch->beginTime = clock();
        for (int n = 0; n < numOfPackages; n++) {
            cout << "Paczka nr " << n << "  czeka na semafor" << endl;
            log_packages[n].beginTime = clock(); // Uwaga! doliczamy czas oczekiwania na zadania!!!!!!!
            sem_wait(&semTable[n]);
            cout << "Paczka nr " << n << " rozsylana" << endl;
            //Rozsyłamy zadania
            for (long i = 0; i < sizeOfPackage; ++i) {
                ff_send_out(new Task(i, n, modelSHA));
            }
            cout << "Paczka nr " << n << " - wszystkie zadania zostaly rozeslane" << endl;
            sem_post(&semTable[n]);
        }
        //log_batch->endTime = clock();
        pthread_join(time_thread, NULL);

        delete[] LicznikJedynek;
        delete[] mdString;
        return EOS;
    }//koniec serwisu

};

struct EmitterU : ff_node_t<Task> {
    //MODEL Z ROZKLADEMJEDNOSTAJNYM  ROZSYLEM ZADAN -LOSOWYM 
    //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek, 

private:
    unsigned int processingTime, numOfPackages, sizeOfPackage;
public:

    EmitterU(unsigned int _processingTime = 3600, unsigned int _numOfPackages = 12, unsigned int _sizeOfPackage = 10)
    : processingTime(_processingTime), numOfPackages(_numOfPackages), sizeOfPackage(_sizeOfPackage) {
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
        return ( rand() % ((processingTime / 60) - (processingTime / (60 * numOfPackages))) + 1);
    }

    Task *svc(Task *) override {
        cout << "Start EmitterU\n";
        //down_sem(numOfPackages);
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
        for (int i = 0; i < numOfPackages; i++) {
            for (int j = i + 1; j < numOfPackages; j++) {
                sleepTable[j] -= sleepTable[i];
            }
        }
        wylosowanych = 0;
        do {
            cout << sleepTable[ wylosowanych ] << "\n";
            wylosowanych++;
        } while (wylosowanych < numOfPackages);

        //PO TEJ PROCEDURZE MAMY 12 LICZB Z ZAKRESU 1..60  

        //CZYLI ROZSYL NASTEPUJE W WYLOSOWANE[1]*60 ,WYLOSOWANE[2]*60,....WYLOSOWANE[12]*60  SEK 

        //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek,
        TimerData td;
        td.numOfPackages = numOfPackages;
        td.timeTable = sleepTable;
        pthread_create(&(time_thread), NULL, thread_timer, (void*) &td);
        //pthread_create(&(time_thread), NULL, thread_timer, (void*) sleepTable);
        log_batch->beginTime = clock();
        for (int n = 0; n < numOfPackages; n++) {
            cout << "Paczka nr " << n << "  czeka na semafor\n";
            log_packages[n].beginTime = clock(); // Uwaga! doliczamy czas oczekiwania na zadania!!!!!!!
            sem_wait(&semTable[n]);
            cout << "Paczka nr " << n << " rozsylana\n";
            //Rozsyłamy zadania
            for (long i = 0; i < sizeOfPackage; ++i) {
                ff_send_out(new Task(i, n, modelU));
            }
            cout << "Paczka nr " << n << " - wszystkie zadania zostaly rozeslane\n";
            sem_post(&semTable[n]);
        }
        //log_batch->endTime = clock();
        pthread_join(time_thread, NULL);

        delete[] sleepTable;
        return EOS;
    }//koniec serwisu



};

//Poisson  TODO

struct WorkerA : ff_node_t<Task> {

    Task *svc(Task *task) override {
        clock_t begin = clock();
        //worker szybki,nie usypiamy 
        std::cout << "WorkerA has got the task " << task->taskID << "\n";
        /* if  *task % 5==0 licz zadanie małe 
         * if  *task % 5==1 licz zadanie srednie
         * if  *task % 5 =2 licz zadanie duze
         * if  * task % 5 =3  licz zadanie bardzoduze
         * if  *task  % 5 =4 licz zadanie olbrzymie
       
         */
        /* rejestrujemy jakie taski dostał  ten worker w kolejnych rozdaniach */
        int taskTime = 0;
        if (task->taskID % 5 == 0) taskTime = 60;
        if (task->taskID % 5 == 1) taskTime = 120;
        if (task->taskID % 5 == 2) taskTime = 180;
        if (task->taskID % 5 == 3) taskTime = 480;
        if (task->taskID % 5 == 4) taskTime = 600;
        czekaj(taskTime);
        //kiedys bedzie tak:
        //double exec_time = matrix_multiplication(128, 5);
        // pierwszy argument to wymiar macierzy kwadratowej, tutaj 128x128
        // drugi, to ile razy ma się to przemnożyć

        log_tasks.push_back(TaskLog('A', get_my_id(), task->packageID, task->taskID, begin, clock()));

        //        switch (task->emmiterModel) {
        //            case modelREF:
        //                //log_tlREF.push_back(new TaskLog('A', get_my_id(), int _packageID, (int)*task, int taskTime));
        //                break;
        //            case modelBBS:
        //                break;
        //            case modelSHA:
        //                break;
        //            case modelU:
        //                break;
        //        }

        return task;
    }
};

struct WorkerB : ff_node_t<Task> {

    Task *svc(Task *task) override {
        clock_t begin = clock();
        //worker wolniejszy, usypiamy 
        std::cout << "WorkerB has got the task " << task->taskID << "\n";
        czekaj(30);
        /* if  *task % 5==0 licz zadanie małe
         * if  *task % 5==1 licz zadanie srednie
         * if  *task % 5 =2 licz zadanie duze
         * if  * task % 5 =3  licz zadanie bardzoduze
         * if  *task  % 5 =4 licz zadanie olbrzymie
       
         */
        int taskTime = 0;
        if (task->taskID % 5 == 0) taskTime = 60;
        if (task->taskID % 5 == 1) taskTime = 120;
        if (task->taskID % 5 == 2) taskTime = 180;
        if (task->taskID % 5 == 3) taskTime = 480;
        if (task->taskID % 5 == 4) taskTime = 600;
        czekaj(taskTime);
        log_tasks.push_back(TaskLog('B', get_my_id(), task->packageID, task->taskID, begin, clock()));
        //double exec_time = matrix_multiplication(128, 5);
        // pierwszy argument to wymiar macierzy kwadratowej, tutaj 128x128
        // drugi, to ile razy ma się to przemnożyć

        return task;
    }
};

struct WorkerC : ff_node_t<Task> {

    Task *svc(Task *task) override {
        clock_t begin = clock();
        //worker wolniejszy, usypiamy 

        std::cout << "WorkerC has got the task " << task->taskID << "\n";
        //printf("WorkerC has got the task %ld\n", *((long*) task));
        czekaj(75);
        /* if  *task % 5==0 licz zadanie małe
         * if  *task % 5==1 licz zadanie srednie
         * if  *task % 5 =2 licz zadanie duze
         * if  * task % 5 =3  licz zadanie bardzoduze
         * if  *task  % 5 =4 licz zadanie olbrzymie
       
         */
        int taskTime = 0;
        if (task->taskID % 5 == 0) taskTime = 60;
        if (task->taskID % 5 == 1) taskTime = 120;
        if (task->taskID % 5 == 2) taskTime = 180;
        if (task->taskID % 5 == 3) taskTime = 480;
        if (task->taskID % 5 == 4) taskTime = 600;
        czekaj(taskTime);
        log_tasks.push_back(TaskLog('C', get_my_id(), task->packageID, task->taskID, begin, clock()));
        //double exec_time = matrix_multiplication(128, 5);
        // pierwszy argument to wymiar macierzy kwadratowej, tutaj 128x128
        // drugi, to ile razy ma się to przemnożyć

        return task;
    }
};

struct WorkerD : ff_node_t<Task> {

    Task *svc(Task *task) override {
        clock_t begin = clock();
        //worker wolniejszy, usypiamy 

        std::cout << "WorkerD has got the task " << task->taskID << "\n";
        czekaj(120);
        /* if  *task % 5==0 licz zadanie małe
         * if  *task % 5==1 licz zadanie srednie
         * if  *task % 5 =2 licz zadanie duze
         * if  * task % 5 =3  licz zadanie bardzoduze
         * if  *task  % 5 =4 licz zadanie olbrzymie
       
         */

        int taskTime = 0;
        if (task->taskID % 5 == 0) taskTime = 60;
        if (task->taskID % 5 == 1) taskTime = 120;
        if (task->taskID % 5 == 2) taskTime = 180;
        if (task->taskID % 5 == 3) taskTime = 480;
        if (task->taskID % 5 == 4) taskTime = 600;
        czekaj(taskTime);
        log_tasks.push_back(TaskLog('D', get_my_id(), task->packageID, task->taskID, begin, clock()));
        //double exec_time = matrix_multiplication(128, 5);
        // pierwszy argument to wymiar macierzy kwadratowej, tutaj 128x128
        // drugi, to ile razy ma się to przemnożyć

        return task;
    }
};

struct WorkerE : ff_node_t<Task> {

    Task *svc(Task *task) override {
        clock_t begin = clock();
        //worker wolniejszy, usypiamy 

        std::cout << "WorkerE has got the task " << task->taskID << "\n";
        czekaj(150);
        /* if  *task % 5==0 licz zadanie małe
         * if  *task % 5==1 licz zadanie srednie
         * if  *task % 5 =2 licz zadanie duze
         * if  * task % 5 =3  licz zadanie bardzoduze
         * if  *task  % 5 =4 licz zadanie olbrzymie
       
         */

        int taskTime = 0;
        if (task->taskID % 5 == 0) taskTime = 60;
        if (task->taskID % 5 == 1) taskTime = 120;
        if (task->taskID % 5 == 2) taskTime = 180;
        if (task->taskID % 5 == 3) taskTime = 480;
        if (task->taskID % 5 == 4) taskTime = 600;
        czekaj(taskTime);
        log_tasks.push_back(TaskLog('E', get_my_id(), task->packageID, task->taskID, begin, clock()));
        //double exec_time = matrix_multiplication(128, 5);
        // pierwszy argument to wymiar macierzy kwadratowej, tutaj 128x128
        // drugi, to ile razy ma się to przemnożyć

        return task;
    }
};


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

#endif /* RREMITTER_H */

