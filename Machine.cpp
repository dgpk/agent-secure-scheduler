/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include"Machine.h"

ostream& operator<<(ostream& os, const Machine& m) {
    os << "ID: " << m.idMachine << "\tFitness: " << m.fitness << "\tSched: ";
    for (std::vector<int>::const_iterator i = m.scheduler.begin(); i != m.scheduler.end(); ++i)
        os << *i << ' ';
    return os;
}

void Machine::countFitness(vector<vector<double> > &matrixETC) {
    fitness = 0.0;

    for (int i = 0; i < scheduler.size(); i++)
        fitness += matrixETC[idMachine][scheduler[i]];
}