/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Machine.h
 * Author: daniel
 *
 * Created on 10 lipca 2016, 00:48
 */

#ifndef MACHINE_H
#define MACHINE_H

#include <cstdlib>
#include <iostream>
#include <vector>

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

#endif /* MACHINE_H */

