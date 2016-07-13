/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   main.cpp
 * Author: daniel
 *
 * Created on 19 czerwca 2016, 23:36
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include <stdlib.h>
#include <functional>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <chrono>
#include <ctime>
#include <cfloat>
#include <algorithm>
#include "GASched.h"


using namespace std;

/*
 *
 */


// schedulerowanie.cpp : Defines the entry point for the console application.
//


// parametry
const int machineNumber = 20;
const int taskNumber = 200;
const int taskPerMachine = (taskNumber / machineNumber);

int maxIter = 10000; //zmiana ze 100
double averageValue = 50.0;
double sigma = 19.0;

vector <vector <double>> matrixETC(machineNumber, vector<double>(taskNumber));

// prototypy funkcji
void MainFunc();
void ETCGenerator(vector <double> &vectETC, int taskNumber, int machineNumber);





double CountFitness(vector<Machine> &machineVect, vector<vector<double> > &matrixETC) {
    double max = 0.0;
    for (int i = 0; i < machineNumber; i++) {
        machineVect[i].countFitness(matrixETC);

        if (max < machineVect[i].fitness)
            max = machineVect[i].fitness;
    }

    return max;
}

double GetTaskTime(int idMachine, int idTask){
    return matrixETC[idMachine][idTask];
}


//int main(int argc, char** argv) {
//    MainFunc();
//
//    getchar();
//    //system("Pause");
//    return 0;
//}

//void ETCGenerator(vector <double> &vectETC, int taskNumber, int machineNumber) {
//    default_random_engine generator;
//    normal_distribution<double> distribution(averageValue, sigma);
//    fstream file;
//
//    file.open("ETC.txt", ios::out);
//
//    double generatedNumb = 0.0;
//    int iterator = 0;
//
//    while (iterator < taskNumber) {
//        generatedNumb = distribution(generator);
//
//        if (generatedNumb >= 1.0 || generatedNumb <= 100.0) {
//            file << setw(8) << fixed << setprecision(2) << generatedNumb;
//            iterator++;
//        }
//    }
//
//    file << endl;
//
//    //generacja reszty czasow
//    double w = 0.5 / 19.0;
//    double machineSpeedValue = 1.0 - w;
//
//    for (int i = 1; i < machineNumber; i++) {
//        for (int j = 0; j < taskNumber; j++)
//            file << setw(8) << fixed << setprecision(2) << vectETC[j] * machineSpeedValue;
//
//        file << endl;
//        machineSpeedValue -= w;
//    }
//
//    file.close();
//}

bool Match(const Machine &m1, const Machine & m2) {
    return m1.fitness < m2.fitness;
}

void LoadMatrixETC(vector<vector<double> > &matrixETC) {
    fstream file;
    file.open("ETC.txt", ios::in);

    for (int i = 0; i < machineNumber; i++)
        for (int j = 0; j < taskNumber; j++)
            file >> matrixETC[i][j];

    file.close();
}

void LoadVectETC(vector<double>&vectETC) {
    fstream file;
    file.open("ETC.txt", ios::in);

    for (int i = 0; i < taskNumber; i++)
        file >> vectETC[i];

    file.close();
}

void Init(vector<double> &vectETC, vector<Machine> &machineVect) {
    //cout << machineVect.capacity() << endl;
    //machineVect.clear();
    //cout << machineVect.capacity() << endl;
    srand(time(NULL));
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();

    vector<int> Init;

    for (int i = 0; i < taskNumber; i++)
        Init.push_back(i);

    shuffle(Init.begin(), Init.end(), default_random_engine(seed));

    //std::random_shuffle ( Init.begin(), Init.end() );  // Linux
    
    int actTaskNumber = 0;

    for (int i = 0; i < machineNumber; i++) {
        machineVect[i].idMachine = i;

        for (int j = 0; j < taskPerMachine; j++) {
            machineVect[i].scheduler.push_back(Init[actTaskNumber]);
            actTaskNumber++;
        }
    }
}

void Crossing(int j, int k, int point, vector<Machine> &machineVect) {
    int tmp;

    for (int i = point; i < taskPerMachine; i++) {
        tmp = machineVect[j].scheduler[i];
        machineVect[j].scheduler[i] = machineVect[k].scheduler[i];
        machineVect[k].scheduler[i] = tmp;
    }
}

void PrintSolution(vector<Machine> &machineVect) {
    for (std::vector<Machine>::const_iterator i = machineVect.begin(); i != machineVect.end(); ++i)
        cout << *i << '\n';
}

vector<Machine> PrepareSchedule() {
//    vector<Machine> machineVect(machineNumber);
    vector<Machine> tmpVect(machineNumber);

    //vector <vector <double>> matrixETC(machineNumber, vector<double>(taskNumber));
    vector <double> vectETC(taskNumber);
    vector<Machine> bestIndivuals(machineNumber);

    //ETCGenerator(taskNumber, machineNumber);										To jest wazne - gdy to odkomentujemy to stworzy sie nowa macierz ETC sluzaca dotestow,lepiej nie odkomentowywac bo juz zostala stworzona

    int j = 0;
    int k = 0;
    int point;
    double prev_value = 0; // uzywany do zapamietywania wartosci poprzedniej

    LoadMatrixETC(matrixETC);
    LoadVectETC(vectETC);
    

    double actTime = 0.0;
    //int bestIter = 0;
    double bestTime = DBL_MAX;
    for(int island = 0; island < 10; island ++)
    {
        vector<Machine> machineVect(machineNumber);
        Init(vectETC, machineVect);
        cout << "Island: " << island << endl;
    for (int i = 0; i < maxIter; i++) //pętla główna programu, w niej się wykonują krzyżowania, sortowania itd
    {
        actTime = CountFitness(machineVect, matrixETC);
        if (actTime < bestTime) {
            for (int j = 0; j < machineNumber; j++)
                bestIndivuals[j] = machineVect[j];

            //bestIter = i;
            bestTime = actTime;
        }
        //SELECKCJA KRZYŻOWANIE ELITARYZM
        sort(machineVect.begin(), machineVect.end(), Match);

        for (int i = 0; i < machineNumber; i++) //wyzerowanie flagi uzycia
            machineVect[i].isUsed = 0;

        int num = 4; // najoptymalniej 1-2

        for (int i = machineNumber - num; i < machineNumber; i++) // krzyzuje najszybsze z najwolniejszymi
        {
            do {
                j = rand() % num;
            } while (machineVect[j].isUsed);
            do {
                k = rand() % num + (machineNumber - num);
            } while (machineVect[k].isUsed);

            point = rand() % taskPerMachine;
            Crossing(j, k, point, machineVect);
            machineVect[j].isUsed = 1; // ustawienie flagi aby nie brano już tych maszyn
            machineVect[k].isUsed = 1;

        }
        //for (int i = 0; i < num; i++)		//krzyzuje maszyny ze srodka
        //{
        //	do
        //	{
        //		j = rand() % (machineNumber - num) + num;
        //	} while (machineVect[j].isUsed);
        //	do
        //	{
        //		k = rand() % (machineNumber / 2 - num) + machineNumber / 2;
        //	} while (machineVect[k].isUsed || j == k);

        //	point = rand() % taskPerMachine;
        //	Crossing(j, k, point, machineVect);
        //	machineVect[j].isUsed = 1;
        //	machineVect[k].isUsed = 1;
        //}

        sort(bestIndivuals.begin(), bestIndivuals.end(), Match);

        if (prev_value != bestIndivuals[9].fitness) // wyswietli sie tylko wtedy gdy nastapia jakies zmiany
        {
            cout << "Najsz:  " << bestIndivuals[0].fitness
                    << " avg:  " << bestIndivuals[9].fitness // srodkowy
                    << " Najw:  " << bestIndivuals[19].fitness // roznica
                    << " DIFF:   " << abs(bestIndivuals[0].fitness - bestIndivuals[19].fitness)
                    << " Iter:   " << i
                    << endl;
        }
        prev_value = bestIndivuals[9].fitness;
    }
    }
    cout << endl;
    PrintSolution(bestIndivuals);
    return bestIndivuals;
}
