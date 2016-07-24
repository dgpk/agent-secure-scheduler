/* 
 * File:   global_fun.h
 * Author: daniel
 *
 * Created on 4 styczeń 2016, 22:04
 */

#pragma once

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
#include <openssl/rand.h>
#include <openssl/bn.h>

#define ulong unsigned long int


////////////////////////////////////////

extern sem_t *semTable; // tablica semaforów

enum model {
    modelREF,
    modelBBS,
    modelSHA,
    modelU,
    modelGAREF,
    modelGABBS,
    modelGASHA,
    modelGAU
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

struct ScheduleLog {
    //id pakietu
    int packageID;
    //czas generowania haromonogramu
    clock_t beginTime = 0;
    clock_t endTime = 0;
    //harmonogram
    vector<Machine> schedule;

    ScheduleLog(int _packageID, clock_t _beginTime, clock_t _endTime, vector<Machine> &_schedule)
    : packageID(_packageID), beginTime(_beginTime), endTime(_endTime) {
        schedule.reserve(_schedule.size());
        copy(_schedule.begin(), _schedule.end(), back_inserter(schedule));
        //cout << "Schedule LOG \n";
        //for (std::vector<Machine>::const_iterator i = schedule.begin(); i != schedule.end(); ++i)
        //cout << *i << endl;
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



extern vector<TaskLog> log_tasks;
extern vector<PackageLog> log_packages;
extern BatchLog *log_batch;
extern vector<ScheduleLog> log_schedule;


//vector<TaskLog> log_tlREF, log_tlBBS, log_tlSHA, log_tlU;
//vector<PackageLog> log_plREF, log_plBBS, log_plSHA, log_plU;
//BatchLog *log_blREF, *log_blBBS, *log_blSHA, *log_blU;



void init_logs(int numOfPackages);

void exportToCSV(char *model, unsigned int processingTime, unsigned int numOfPackages, unsigned int sizeOfPackage);

void czekaj(int iSekundy);

void init_sem(unsigned int numOfPackages);

void down_sem(unsigned int numOfPackages);

void destroy(unsigned int numOfPackages);

void* thread_timer(void *ptr);

///////////////////////////////////

double matrix_multiplication(size_t size, size_t ntimes);

//char* get_SHA512(char *str);
void get_SHA512(char* str, char* res);

ulong* generateBBSKey(ulong N);

void gen_random(char *s, const int len);

const char* hex_char_to_bin(char c);

#endif /* GLOBAL_FUN_H */

