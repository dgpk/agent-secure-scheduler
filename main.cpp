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
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <fstream>
#include <openssl/ssl.h> 
#include <ff/farm.hpp>
#include <ff/pipeline.hpp>
#include "global_fun.h"
#include "GASched.h"
#include "RREmitter.h"


using namespace std;
using namespace ff;

#define REF
#define BBS
#define SHA
#define U

//#define ScreenLog

#define ulong unsigned long int
sem_t *semTable; // tablica semaforów
vector<TaskLog> log_tasks;
vector<PackageLog> log_packages;
BatchLog *log_batch = NULL;
vector<ScheduleLog> log_schedule;

// Maszyna "niescharakteryzowana"
// jej charakterystyka jest skladowa macierzy ETC

struct WorkerETC : ff_node_t<Task> {

    Task *svc(Task *task) override {
        clock_t begin = clock();
#ifdef ScreenLog
        std::cout << "Machine " << get_my_id() << " has got the task "
                << task->taskID << " that will take " << task->expectedExecTime
                << "sec \n";
#endif
        czekaj(task->expectedExecTime);

        log_tasks.push_back(TaskLog('G', get_my_id(), task->packageID, task->taskID, begin, clock()));

        return task;
    }
};

struct Collector : ff_node_t<Task> {
    ff_gatherer * const gt;
    unsigned int numOfPackages;
    unsigned int sizeOfPackage;
    int *controlTable;

    Collector(ff_gatherer * const gt, unsigned int _numOfPackages, unsigned int _sizeOfPackage)
    : gt(gt), numOfPackages(_numOfPackages), sizeOfPackage(_sizeOfPackage) {
        controlTable = new int[numOfPackages]();
    }

    ~Collector() {
        delete [] controlTable;
    }

    Task *svc(Task *task) override {
        //std::cout << "received task from Worker " << gt->get_channel_id() << "\n";
        controlTable[task->packageID]++;
        if (controlTable[task->packageID] == sizeOfPackage) {
            // mamy ostatnie zadanie
            log_packages[task->packageID].endTime = clock();
            cout << "paczka: " << task->packageID << "\tend time: " << log_packages[task->packageID].endTime << "\n";
        }
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





// GA start

void create_workersETC(std::vector<std::unique_ptr<ff_node> > *Workers, int nworkers) {
    for (int i = 1; i <= nworkers; ++i) {
        Workers->push_back(make_unique<WorkerETC>());
        //cout << "Worker ETC created" << endl;
    }
}

struct EmitterGAREF : ff_node_t<Task> {
    // składowe publiczne struktury
    //unsigned int wait_time = 300;
    // po rozesłaniu paczki (uwaga! nie po wykonaniu wszystkich zadań) czekamy tyle sekund (tut. 5 minut)
private:
    unsigned int processingTime, numOfPackages;
    ff_loadbalancer * lb;
public:

    EmitterGAREF(unsigned int _processingTime = 3600,
            unsigned int _numOfPackages = 12, ff_loadbalancer * const _lb = NULL)
    : processingTime(_processingTime), numOfPackages(_numOfPackages), lb(_lb) {
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
            // generujemy harmonogram
            clock_t sched_begin = clock();
            vector<Machine> schedule = PrepareSchedule();
            clock_t sched_end = clock();
            log_schedule.push_back(ScheduleLog(n, sched_begin, sched_end, schedule));
            vector<Machine>::iterator machineIt;
            vector<int>::iterator taskIt;
            cout << "Paczka (batch/harmonogram) nr " << n << "  czeka na semafor" << endl;
            //Rozsyłamy zadania
            log_packages[n].beginTime = clock();
            cout << "start time: " << log_packages[n].beginTime << "\n";
            sem_wait(&semTable[n]);
            cout << "Paczka nr " << n << " rozsylana" << endl;

            for (machineIt = schedule.begin(); machineIt != schedule.end(); ++machineIt) {
                for (taskIt = machineIt->scheduler.begin(); taskIt != machineIt->scheduler.end(); ++taskIt) {
                    lb->ff_send_out_to(new Task(*taskIt, n, modelGAREF, GetTaskTime(machineIt->idMachine, *taskIt)), machineIt->idMachine);
                }
            }


            cout << "Paczka nr " << n << " - wszystkie zadania zostaly rozeslane" << endl;
            sem_post(&semTable[n]); // czy to nie powinno isc do collectora?
        }
        pthread_join(time_thread, NULL);
        //log_batch->endTime = clock();
        return EOS;
    }
};

struct EmitterGABBS : ff_node_t<Task> {
private:
    unsigned int processingTime, numOfPackages;
    ff_loadbalancer * lb;
public:

    EmitterGABBS(unsigned int _processingTime = 3600, unsigned int _numOfPackages = 12, ff_loadbalancer * const _lb = NULL)
    : processingTime(_processingTime), numOfPackages(_numOfPackages), lb(_lb) {
        srand((unsigned int) time((time_t *) NULL)); // korzystamy z funkcji rand  // ZA KAŻDYM RAZEM WYLOSUJEMY INNY ROZKŁAD  !!!!!!!!!!!!!!
    }
    //MODEL Z BBSOWYM ROZSYLEM ZADAN - LOSOWYM ZGODNIE Z BBS 
    //pojedynczy serwis rozsyla pakiet zadan x12 razy wciagu godziny:60*60 sek=3600sek, ale robi to zgodnie zmodelem BBS


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
            // generujemy harmonogram
            clock_t sched_begin = clock();
            vector<Machine> schedule = PrepareSchedule();
            clock_t sched_end = clock();
            log_schedule.push_back(ScheduleLog(n, sched_begin, sched_end, schedule));
            vector<Machine>::iterator machineIt;
            vector<int>::iterator taskIt;
            cout << "Paczka nr " << n << "  czeka na semafor" << endl;
            log_packages[n].beginTime = clock(); // Uwaga! doliczamy czas oczekiwania na zadania!!!!!!!
            sem_wait(&semTable[n]);
            cout << "Paczka nr " << n << " rozsylana" << endl;
            //Rozsyłamy zadania
            for (machineIt = schedule.begin(); machineIt != schedule.end(); ++machineIt) {
                for (taskIt = machineIt->scheduler.begin(); taskIt != machineIt->scheduler.end(); ++taskIt) {
                    lb->ff_send_out_to(new Task(*taskIt, n, modelGABBS, GetTaskTime(machineIt->idMachine, *taskIt)), machineIt->idMachine);
                }
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

struct EmitterGASHA : ff_node_t<Task> {
private:
    unsigned int processingTime, numOfPackages;
    ff_loadbalancer * lb;
public:

    EmitterGASHA(unsigned int _processingTime = 3600, unsigned int _numOfPackages = 12, ff_loadbalancer * const _lb = NULL)
    : processingTime(_processingTime), numOfPackages(_numOfPackages), lb(_lb) {
        srand((unsigned int) time((time_t *) NULL)); // korzystamy z funkcji rand  // ZA KAŻDYM RAZEM WYLOSUJEMY INNY ROZKŁAD  !!!!!!!!!!!!!!
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
            // generujemy harmonogram
            clock_t sched_begin = clock();
            vector<Machine> schedule = PrepareSchedule();
            clock_t sched_end = clock();
            log_schedule.push_back(ScheduleLog(n, sched_begin, sched_end, schedule));
            vector<Machine>::iterator machineIt;
            vector<int>::iterator taskIt;
            cout << "Paczka nr " << n << "  czeka na semafor" << endl;
            log_packages[n].beginTime = clock(); // Uwaga! doliczamy czas oczekiwania na zadania!!!!!!!
            sem_wait(&semTable[n]);
            cout << "Paczka nr " << n << " rozsylana" << endl;
            //Rozsyłamy zadania
            for (machineIt = schedule.begin(); machineIt != schedule.end(); ++machineIt) {
                for (taskIt = machineIt->scheduler.begin(); taskIt != machineIt->scheduler.end(); ++taskIt) {
                    lb->ff_send_out_to(new Task(*taskIt, n, modelGASHA, GetTaskTime(machineIt->idMachine, *taskIt)), machineIt->idMachine);
                }
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

struct EmitterGAU : ff_node_t<Task> {
    //MODEL Z ROZKLADEMJEDNOSTAJNYM  ROZSYLEM ZADAN -LOSOWYM 
    //pojedynczy serwis rozsyla 100 zadan x12 razy wciagu godziny:60*60 sek=3600sek, 

private:
    unsigned int processingTime, numOfPackages;
    ff_loadbalancer * lb;
public:

    EmitterGAU(unsigned int _processingTime = 3600, unsigned int _numOfPackages = 12, ff_loadbalancer * const _lb = NULL)
    : processingTime(_processingTime), numOfPackages(_numOfPackages), lb(_lb) {
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
            // generujemy harmonogram
            clock_t sched_begin = clock();
            vector<Machine> schedule = PrepareSchedule();
            clock_t sched_end = clock();
            log_schedule.push_back(ScheduleLog(n, sched_begin, sched_end, schedule));
            vector<Machine>::iterator machineIt;
            vector<int>::iterator taskIt;
            cout << "Paczka nr " << n << "  czeka na semafor\n";
            log_packages[n].beginTime = clock(); // Uwaga! doliczamy czas oczekiwania na zadania!!!!!!!
            sem_wait(&semTable[n]);
            cout << "Paczka nr " << n << " rozsylana\n";
            //Rozsyłamy zadania
            for (machineIt = schedule.begin(); machineIt != schedule.end(); ++machineIt) {
                for (taskIt = machineIt->scheduler.begin(); taskIt != machineIt->scheduler.end(); ++taskIt) {
                    lb->ff_send_out_to(new Task(*taskIt, n, modelGAU, GetTaskTime(machineIt->idMachine, *taskIt)), machineIt->idMachine);
                }
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

// GA stop

int main(int argc, char **argv) {
    int choice = 0;
    while (choice == 0) {
        cout << "Wybierz doswiadczenie:" << endl;
        cout << "1 - round robin\n2 - GA\n3 - only schedule with security factor" << endl;
        cin >> choice;
        if (choice < 1 || choice > 3)
            choice = 0;
    }

    switch (choice) {
        case 1:
        {

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
            unsigned int processingTime = 3600; // Godzina na wykonanie wszystkich paczek
            unsigned int numOfPackages = 12; // Liczba paczek na processingTime
            unsigned int sizeOfPackage = 10; // liczba zadań w paczce



            init_sem(numOfPackages);
            down_sem(numOfPackages);
            std::vector<std::unique_ptr<ff_node> > Workers;

            cout << "Czas start\n";
            clock_t begin = clock();


#ifdef REF
            cout << "FARMA Z EMITEREM REFERENCYJNYM" << endl;
            init_logs(numOfPackages);
            create_workers(&Workers, nworkers);
            ff_Farm<Task> farm(std::move(Workers));
            farm.set_scheduling_ondemand(); // set auto scheduling
            Emitter E(processingTime, numOfPackages, sizeOfPackage, 300);
            Collector C(farm.getgt(), numOfPackages, sizeOfPackage);
            farm.add_emitter(E); // add the specialized emitter
            farm.add_collector(C);
            if (farm.run_and_wait_end() < 0) error("running  REFERENCE farm");
            log_batch->endTime = clock();
            exportToCSV("REF", processingTime, numOfPackages, sizeOfPackage);
#endif

#ifdef BBS
            cout << "FARMA Z EMITEREM BBS" << endl;
            init_logs(numOfPackages);
            //std::vector<std::unique_ptr<ff_node> > WorkersBBS;
            create_workers(&Workers, nworkers);
            ff_Farm<Task> farmBBS(std::move(Workers));
            farmBBS.set_scheduling_ondemand(); // set auto scheduling
            EmitterBBS EBBS(processingTime, numOfPackages, sizeOfPackage);
            Collector CBBS(farmBBS.getgt(), numOfPackages, sizeOfPackage);
            farmBBS.add_emitter(EBBS); // add the specialized emitter
            farmBBS.add_collector(CBBS);
            if (farmBBS.run_and_wait_end() < 0) error("running BBS  farm");
            log_batch->endTime = clock();
            exportToCSV("BBS", processingTime, numOfPackages, sizeOfPackage);
#endif

#ifdef SHA
            cout << "FARMA Z EMITEREM SHA" << endl;
            init_logs(numOfPackages);
            //std::vector<std::unique_ptr<ff_node> > WorkersSHA;
            create_workers(&Workers, nworkers);
            ff_Farm<Task> farmSHA(std::move(Workers));
            farmSHA.set_scheduling_ondemand(); // set auto scheduling
            EmitterSHA ESHA(processingTime, numOfPackages, sizeOfPackage);
            Collector CSHA(farmSHA.getgt(), numOfPackages, sizeOfPackage);
            farmSHA.add_emitter(ESHA); // add the specialized emitter
            farmSHA.add_collector(CSHA);
            if (farmSHA.run_and_wait_end() < 0) error("running SHA farm");
            log_batch->endTime = clock();
            exportToCSV("SHA", processingTime, numOfPackages, sizeOfPackage);
#endif

#ifdef U
            cout << "FARMA Z EMITEREM JEDNOSTAJNYM" << endl;
            init_logs(numOfPackages);
            create_workers(&Workers, nworkers);
            ff_Farm<Task> farmU(std::move(Workers));
            farmU.set_scheduling_ondemand(); // set auto scheduling
            EmitterU EU(processingTime, numOfPackages, sizeOfPackage);
            Collector CU(farmU.getgt(), numOfPackages, sizeOfPackage);
            farmU.add_emitter(EU); // add the specialized emitter
            farmU.add_collector(CU);
            if (farmU.run_and_wait_end() < 0) error("running UNITED farm");
            log_batch->endTime = clock();
            exportToCSV("U", processingTime, numOfPackages, sizeOfPackage);
#endif

            clock_t end = clock();
            double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
            cout << "Czas stop: " << elapsed_secs << " [s]" << endl;
            cout << "Press enter to continue ..." << endl;
            cin.get();
            destroy(numOfPackages);
            break;
        }
        case 2:
        {
            //vector<Machine> schedule = PrepareSchedule();
            initETCMatrix();
            int GAnworkers = 20;
            unsigned int GAprocessingTime = 300; //3600; // Godzina na wykonanie wszystkich paczek
            unsigned int GAnumOfPackages = 2; //12; // Liczba paczek na processingTime
            unsigned int GAsizeOfPackage = 200; // liczba zadań w paczce



            init_sem(GAnumOfPackages);
            down_sem(GAnumOfPackages);
            std::vector<std::unique_ptr<ff_node> > GAWorkers;

            cout << "Czas start\n";
            clock_t begin = clock();


#ifdef REF
            cout << "FARMA GENETYCZNA Z EMITEREM REFERENCYJNYM" << endl;
            init_logs(GAnumOfPackages);
            create_workersETC(&GAWorkers, GAnworkers);
            ff_Farm<Task> farmGAREF(std::move(GAWorkers));
            //farm.set_scheduling_ondemand(); // set auto scheduling
            EmitterGAREF EGAREF(GAprocessingTime, GAnumOfPackages, farmGAREF.getlb());
            Collector CGAREF(farmGAREF.getgt(), GAnumOfPackages, GAsizeOfPackage);
            farmGAREF.add_emitter(EGAREF); // add the specialized emitter
            farmGAREF.add_collector(CGAREF);
            if (farmGAREF.run_and_wait_end() < 0) error("running GA REFERENCE farm");
            log_batch->endTime = clock();
            exportToCSV("GAREF", GAprocessingTime, GAnumOfPackages, GAsizeOfPackage);
#endif

#ifdef BBS
            cout << "FARMA GENETYCZNA Z EMITEREM BBS" << endl;
            if (GAnumOfPackages > 1) {
                init_logs(GAnumOfPackages);
                create_workersETC(&GAWorkers, GAnworkers);
                ff_Farm<Task> farmGABBS(std::move(GAWorkers));
                //farm.set_scheduling_ondemand(); // set auto scheduling
                EmitterGABBS EGABBS(GAprocessingTime, GAnumOfPackages, farmGABBS.getlb());
                Collector CGABBS(farmGABBS.getgt(), GAnumOfPackages, GAsizeOfPackage);
                farmGABBS.add_emitter(EGABBS); // add the specialized emitter
                farmGABBS.add_collector(CGABBS);
                if (farmGABBS.run_and_wait_end() < 0) error("running GA BBS farm");
                log_batch->endTime = clock();
                exportToCSV("GABBS", GAprocessingTime, GAnumOfPackages, GAsizeOfPackage);
            } else {
                cout << "GAnumOfPackages nie jest wiekszy od 1" << endl;
            }
#endif

#ifdef SHA
            cout << "FARMA GENETYCZNA Z EMITEREM SHA" << endl;
            init_logs(GAnumOfPackages);
            create_workersETC(&GAWorkers, GAnworkers);
            ff_Farm<Task> farmGASHA(std::move(GAWorkers));
            //farm.set_scheduling_ondemand(); // set auto scheduling
            EmitterGASHA EGASHA(GAprocessingTime, GAnumOfPackages, farmGASHA.getlb());
            Collector CGASHA(farmGASHA.getgt(), GAnumOfPackages, GAsizeOfPackage);
            farmGASHA.add_emitter(EGASHA); // add the specialized emitter
            farmGASHA.add_collector(CGASHA);
            if (farmGASHA.run_and_wait_end() < 0) error("running GA SHA farm");
            log_batch->endTime = clock();
            exportToCSV("GASHA", GAprocessingTime, GAnumOfPackages, GAsizeOfPackage);
#endif

#ifdef U
            cout << "FARMA GENETYCZNA Z EMITEREM JEDNOSTAJNYM" << endl;
            init_logs(GAnumOfPackages);
            create_workersETC(&GAWorkers, GAnworkers);
            ff_Farm<Task> farmGAU(std::move(GAWorkers));
            //farm.set_scheduling_ondemand(); // set auto scheduling
            EmitterGAU EGAU(GAprocessingTime, GAnumOfPackages, farmGAU.getlb());
            Collector CGAU(farmGAU.getgt(), GAnumOfPackages, GAsizeOfPackage);
            farmGAU.add_emitter(EGAU); // add the specialized emitter
            farmGAU.add_collector(CGAU);
            if (farmGAU.run_and_wait_end() < 0) error("running GA U farm");
            log_batch->endTime = clock();
            exportToCSV("GAU", GAprocessingTime, GAnumOfPackages, GAsizeOfPackage);
#endif
            clock_t end = clock();
            double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
            cout << "Czas stop: " << elapsed_secs << " [s]" << endl;
            cout << "Press enter to continue ..." << endl;
            cin.get();
            destroy(GAnumOfPackages);
            break;
        }
        case 3:
        {
            initETCMatrix();
            double securityFactor;
            int repeats, epochs;
            cout << "Enter security factor (eg. 0.25): ";
            cin >> securityFactor;
            cout << "Enter the number of epoches (eg. 10000): ";
            cin >> epochs;
            cout << "How many times repeat the genetic process? (eg. 1000)\n";
            cin >> repeats;
            vector<double> makespans;
            for(int i=0; i<repeats; i++)
                makespans.push_back(PrepareSecureSchedule(epochs, securityFactor));
            
            for (vector<double>::iterator it = makespans.begin(); it != makespans.end(); ++it) {
                cout << *it << "\n";
            }
        }
        case 4:
        {
            //TODO - security levels
        }
    }



    return 0;
}