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
#include "GASched.h"
#include "RREmitter.h"
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




// Maszyna "niescharakteryzowana"
// jej charakterystyka jest skladowa macierzy ETC

struct WorkerETC : ff_node_t<Task> {

    Task *svc(Task *task) override {
        clock_t begin = clock();

        std::cout << "Machine " << get_my_id() << " has got the task "
                << task->taskID << " that will take " << task->expectedExecTime
                << "sec \n";
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
        cout << "Worker ETC created" << endl;
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
            vector<Machine> schedule = PrepareSchedule();
            vector<Machine>::iterator machineIt;
            vector<int>::iterator taskIt;
            cout << "Paczka (batch/harmonogram) nr " << n << "  czeka na semafor" << endl;
            //Rozsyłamy zadania
            log_packages[n].beginTime = clock();
            sem_wait(&semTable[n]);
            cout << "Paczka nr " << n << " rozsylana" << endl;

            for (machineIt = schedule.begin(); machineIt != schedule.end(); ++machineIt) {
                for (taskIt = machineIt->scheduler.begin(); taskIt != machineIt->scheduler.end(); ++taskIt) {
                    lb->ff_send_out_to(new Task(*taskIt, n, modelGAREF, GetTaskTime(machineIt->idMachine, *taskIt)), machineIt->idMachine);
                }
            }


            cout << "Paczka nr " << n << " - wszystkie zadania zostaly rozeslane" << endl;
            sem_post(&semTable[n]);
        }
        pthread_join(time_thread, NULL);
        //log_batch->endTime = clock();
        return EOS;
    }
};



// GA stop

int main(int argc, char **argv) {
    int choice = 0;
    while (choice == 0) {
        cout << "Wybierz doswiadczenie:" << endl;
        cout << "1 - round robin\n2 - GA" << endl;
        cin >> choice;
        if (choice < 1 || choice > 2)
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
            
                        int GAnworkers = 20;
            unsigned int GAprocessingTime = 3600; // Godzina na wykonanie wszystkich paczek
            unsigned int GAnumOfPackages = 12; // Liczba paczek na processingTime



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
            Collector CGAREF(farmGAREF.getgt(), GAnumOfPackages, 200);
            farmGAREF.add_emitter(EGAREF); // add the specialized emitter
            farmGAREF.add_collector(CGAREF);
            if (farmGAREF.run_and_wait_end() < 0) error("running GA REFERENCE farm");
            log_batch->endTime = clock();
            exportToCSV("REFGA", GAprocessingTime, GAnumOfPackages, 200);
#endif
            
            clock_t end = clock();
            double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
            cout << "Czas stop: " << elapsed_secs << " [s]" << endl;
            cout << "Press enter to continue ..." << endl;
            cin.get();
            destroy(GAnumOfPackages);
            break;
            
            



        }
    }



    return 0;
}