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
void initETCMatrix();
double PrepareSecureSchedule(int _maxIter, double _securityFactor);

#endif /* GASCHED_H */

