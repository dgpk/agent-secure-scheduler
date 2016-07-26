/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GASched.h
 * Author: daniel
 *
 * Created on 9 lipca 2016, 23:31
 */


#ifndef GASCHED_H
#define GASCHED_H


using namespace std;

struct SecureStudyLog {
    //int epochId;
    double makespan;
    double bestMakespan;
    double avgTime;
};

class Machine {
public:
    int idMachine; //numer maszyny

    std::vector<int> scheduler;

    double fitness;

    void countFitness(vector< vector <double> > &matrixETC);
    bool isUsed; //flaga potrzebna żeby zapamiętać maszyny które już krzyżowaliśmy
    friend ostream& operator<<(ostream& os, const Machine& m);
};
//extern vector<double> log_makespans;
vector<Machine> PrepareSchedule();
double GetTaskTime(int idMachine, int idTask);
void initETCMatrix(double securityFactor = 1.0);
void PrepareSecureSchedule(int _maxIter, int _numOfCrossingPairs, int _iteration);
void exportSecureStudyToCSV(int maxIter, int numOfCrossingPairs, double securityFactor, int repeats);
double SL_PrepareSchedule(int maxIter, int numOfCrossingPairs, int securityLevel);
double UNI_PrepareSchedule(int maxIter, int numOfCrossingPairs);
void SL_exportToCSV(vector<double> *SL_makespans, vector<double> *UNI_makespans, int epochs, int numOfCrossingPairs, int securityLevels, int repeats);

extern SecureStudyLog **log_securestudy;

#endif /* GASCHED_H */

