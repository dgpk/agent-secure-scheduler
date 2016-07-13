/* 
 * File:   global_fun.h
 * Author: daniel
 *
 * Created on 4 styczeń 2016, 22:04
 */

#ifndef GLOBAL_FUN_H
#define GLOBAL_FUN_H


#include <cstdlib>
#include <iostream>
#include <vector>
#include <time.h>
#include <openssl/ssl.h> 
#include <ff/farm.hpp>
#include <ff/pipeline.hpp>
#include "GASched.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <fstream>

#define ulong unsigned long int


////////////////////////////////////////

sem_t *semTable; // tablica semaforów

enum model {
    modelREF,
    modelBBS,
    modelSHA,
    modelU,
    modelGAREF
};

// charakterystyka zadania

struct Task {
    int taskID;
    int packageID;
    model emmiterModel;
    int expectedExecTime;

    Task(int _taskID, int _packageID, model _emmiterModel,
            int _expectedExecTime = 0)
    : taskID(_taskID), packageID(_packageID),
    emmiterModel(_emmiterModel), expectedExecTime(_expectedExecTime) {
    }
};

// dane przekazywane do watku obliczajacego czas (nadzorcy czasu)

struct TimerData {
    int *timeTable;
    int numOfPackages;
};

// wektor dla każdego modelu

struct TaskLog {
    //worker, package, czas zadania
    char workerType;
    int workerID;
    int packageID;
    int taskNumber;
    //double taskTime;
    clock_t beginTime = 0;
    clock_t endTime = 0;

    TaskLog(char _workerType, int _workerID, int _packageID, int _taskNumber, clock_t _beginTime, clock_t _endTime)
    : workerType(_workerType), workerID(_workerID), packageID(_packageID), taskNumber(_taskNumber), beginTime(_beginTime), endTime(_endTime) {
    }
};

// wektor dla każdego modelu

struct PackageLog {
    //package, czas pakietu
    int packageID;
    //double packageTime;
    clock_t beginTime = 0;
    clock_t endTime = 0;

    PackageLog(int _packageID, clock_t _beginTime, clock_t _endTime) : packageID(_packageID), beginTime(_beginTime), endTime(_endTime) {
    }
};

// obiekt dla każdego modelu

struct BatchLog {
    // czas całego batcha
    //double batchTime;
    clock_t beginTime = 0;
    clock_t endTime = 0;

    BatchLog() : beginTime(0), endTime(0) {
    }

    BatchLog(clock_t _beginTime, clock_t _endTime) : beginTime(_beginTime), endTime(_endTime) {
    }
};


//vector<TaskLog> log_tlREF, log_tlBBS, log_tlSHA, log_tlU;
//vector<PackageLog> log_plREF, log_plBBS, log_plSHA, log_plU;
//BatchLog *log_blREF, *log_blBBS, *log_blSHA, *log_blU;

vector<TaskLog> log_tasks;
vector<PackageLog> log_packages;
BatchLog *log_batch = NULL;

void init_logs(int numOfPackages) {
    if (log_batch)
        delete log_batch;
    log_batch = new BatchLog;
    log_tasks.clear();
    log_packages.clear();
    for (int i = 0; i < numOfPackages; i++) {
        log_packages.push_back(PackageLog(i, 0, 0));
    }
}

void exportToCSV(char *model, unsigned int processingTime, unsigned int numOfPackages, unsigned int sizeOfPackage) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char name[50];
    sprintf(name, "logs/Log_%s_%d-%d-%d_%d.%d.%d.csv", model, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    ofstream fsout;
    fsout.open(name, ios::out);
    if (!fsout.is_open()) {
        cerr << "Blad zapisu do pliku!" << endl;
        return;
    }
    fsout << model << "\n\n";
    fsout << "Parametry\n";
    fsout << "processingTime:;" << processingTime << "\n";
    fsout << "numOfPackages:;" << numOfPackages << "\n";
    fsout << "sizeOfPackage:;" << sizeOfPackage << "\n\n";

    fsout << "Czas wykonywania poszczegolnych zadan [s]\n";
    fsout << "workerType;workerID;packageID;taskNumber;taskTime\n";
    for (vector<TaskLog>::iterator it = log_tasks.begin(); it != log_tasks.end(); ++it) {
        fsout << it->workerType << ";" << it->workerID << ";" << it->packageID << ";" << it->taskNumber << ";" << double(it->endTime - it->beginTime) / CLOCKS_PER_SEC << "\n";
    }
    fsout << "\n";
    fsout << "Czas wykonywania poszczegolnych pakietow [s]\n";
    fsout << "packageID;packageTime\n";
    for (vector<PackageLog>::iterator it = log_packages.begin(); it != log_packages.end(); ++it) {
        fsout << it->packageID << ";" << double(it->endTime - it->beginTime) / CLOCKS_PER_SEC << "\n";
    }

    fsout << "\n";
    fsout << "Czas wykonywania calego wsadu [s]\n";
    fsout << double(log_batch->endTime - log_batch->beginTime) / CLOCKS_PER_SEC << "\n\n";
    fsout << "KONIEC\n";
    fsout.flush();
    fsout.close();
}

//Przygotowanie zadania, które wiemy dokładnieile będzie trwało

void czekaj(int iSekundy) {
    for (clock_t koniec = clock() + iSekundy * CLOCKS_PER_SEC; clock() < koniec;)
        continue;

}

// Inicjalizujemy semafory (funkcja wywoływana raz w main)

void init_sem(unsigned int numOfPackages) {
    semTable = new sem_t[numOfPackages];
    for (int i = 0; i < numOfPackages; i++) {
        sem_init(&semTable[i], 0, 1);
        //taskTable[i] = sizeOfPackage;
    }
    //threads = new pthread_t[numOfPackages];
}

// Opuszczamy semafory (funkcja wywoływana w każdym emmiterze na początku)

void down_sem(unsigned int numOfPackages) {
    for (int i = 0; i < numOfPackages; i++) {
        sem_wait(&semTable[i]);
        //taskTable[i] = sizeOfPackage;
    }
}

// Niszczymy semafory (funkcja wywoływana raz w main - na końcu)

void destroy(unsigned int numOfPackages) {
    for (int i = 0; i < numOfPackages; i++)
        sem_destroy(&semTable[i]);
    delete[] semTable;
}

// Funkcja która zarządza dostępem do paczek zadań 
// czyli zadania przychodzą w określonych odstępach czasu
// Wyświetla informacje o paczce i ile musi czekac na przyjscie
// Po tym czasie podnosi semafor, czyli daje dostep do tych danych
// Po podniesieniu wszystkich semaforów wątek ginie

void* thread_timer(void *ptr) {
    TimerData *td = (TimerData *) ptr;
    for (int i = 0; i < td->numOfPackages; i++) {
        cout << "Paczka " << i << "  Watek czeka " << td->timeTable[i] << endl;
        sleep(td->timeTable[i]);
        sem_post(&semTable[i]);
        cout << "podniesiono semafor " << i << endl;
    }
    pthread_exit(0);

}


///////////////////////////////////

double matrix_multiplication(size_t size, size_t ntimes);

//char* get_SHA512(char *str);
void get_SHA512(char* str, char* res);

ulong* generateBBSKey(ulong N);

void gen_random(char *s, const int len);

#endif	/* GLOBAL_FUN_H */

