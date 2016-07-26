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

//#define ScreenLog

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
const int securityLevels = 4;


double averageValue = 50.0;
double sigma = 19.0;



vector <vector <double>> matrixETC(machineNumber, vector<double>(taskNumber));

// prototypy funkcji
void MainFunc();
void ETCGenerator(vector <double> &vectETC, int taskNumber, int machineNumber);

///// Machine start

ostream& operator<<(ostream& os, const Machine& m) {
    os << "ID: " << m.idMachine << "\tFitness: " << m.fitness << "\tSched: ";
    for (std::vector<int>::const_iterator i = m.scheduler.begin(); i != m.scheduler.end(); ++i)
        os << *i << '(' << GetTaskTime(m.idMachine, *i) << ')' << ' ';
    return os;
}

void Machine::countFitness(vector<vector<double> > &matrixETC) {
    fitness = 0.0;

    for (int i = 0; i < scheduler.size(); i++)
        fitness += matrixETC[idMachine][scheduler[i]];
}


///// Machine stop

double CountFitness(vector<Machine> &machineVect, vector<vector<double> > &matrixETC) {
    double max = 0.0;
    for (int i = 0; i < machineNumber; i++) {
        machineVect[i].countFitness(matrixETC);

        if (max < machineVect[i].fitness)
            max = machineVect[i].fitness;
    }

    return max;
}

double SL_CountFitness(vector<Machine> &machineVect, vector<vector<double> > &matrixETC) {
    double max = 0.0;
    for (int i = 0; i < machineNumber / securityLevels; i++) {
        machineVect[i].countFitness(matrixETC);

        if (max < machineVect[i].fitness)
            max = machineVect[i].fitness;
    }

    return max;
}

double GetTaskTime(int idMachine, int idTask) {
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

void LoadMatrixETC(vector<vector<double> > &matrixETC, double securityFactor = 1.0) {
    fstream file;
    file.open("ETC.txt", ios::in);

    for (int i = 0; i < machineNumber; i++) {
        for (int j = 0; j < taskNumber; j++) {
            file >> matrixETC[i][j];
            matrixETC[i][j] *= securityFactor;
        }
    }

    file.close();
}

//void LoadVectETC(vector<double>&vectETC) {
//    fstream file;
//    file.open("ETC.txt", ios::in);
//
//    for (int i = 0; i < taskNumber; i++)
//        file >> vectETC[i];
//
//    file.close();
//}
//void Init(vector<double> &vectETC, vector<Machine> &machineVect)

void Init(vector<Machine> &machineVect) {
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

void SL_Init(vector<Machine> &machineVect, int securityLevel) {
    srand(time(NULL));
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();

    vector<int> Init;

    for (int i = securityLevel; i < taskNumber; i += securityLevels) {
        Init.push_back(i);
    }

    shuffle(Init.begin(), Init.end(), default_random_engine(seed));

    //std::random_shuffle ( Init.begin(), Init.end() );  //

    int actTaskNumber = 0;

    for (int i = 0; i < machineNumber / securityLevels; i++) {
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

void initETCMatrix(double securityFactor) {
    LoadMatrixETC(matrixETC, securityFactor);
}

vector<Machine> PrepareSchedule() {
    int maxIter = 10000; //zmiana ze 100
    int maxIslands = 1;
    int numOfCrossingPairs = 3;
    vector<Machine> machineVect(machineNumber);
    //vector<Machine> tmpVect(machineNumber);

    //vector <vector <double>> matrixETC(machineNumber, vector<double>(taskNumber));
    //vector <double> vectETC(taskNumber);
    vector<Machine> bestIndivuals(machineNumber);

    //ETCGenerator(taskNumber, machineNumber);										To jest wazne - gdy to odkomentujemy to stworzy sie nowa macierz ETC sluzaca dotestow,lepiej nie odkomentowywac bo juz zostala stworzona

    int j = 0;
    int k = 0;
    int point;
    double prev_value = 0; // uzywany do zapamietywania wartosci poprzedniej

    //LoadMatrixETC(matrixETC); // juz jest wyrzucone na zewnatrz
    //LoadVectETC(vectETC);


    double actTime = 0.0;
    int bestIter = 0;
    double bestTime = DBL_MAX;
    //for(int island = 0; island < 10; island ++)
    //{
    Init(machineVect);
    //cout << "Island: " << island << endl;
    for (int i = 0; i < maxIter; i++) //pętla główna programu, w niej się wykonują krzyżowania, sortowania itd
    {
        actTime = CountFitness(machineVect, matrixETC);
        if (actTime < bestTime) {
            for (int j = 0; j < machineNumber; j++)
                bestIndivuals[j] = machineVect[j];

            bestIter = i;
            bestTime = actTime;
        }
        //SELECKCJA KRZYŻOWANIE ELITARYZM
        sort(machineVect.begin(), machineVect.end(), Match);

        for (int i = 0; i < machineNumber; i++) //wyzerowanie flagi uzycia
            machineVect[i].isUsed = 0;

        //int num = 4; // najoptymalniej 1-2

        for (int i = machineNumber - numOfCrossingPairs; i < machineNumber; i++) // krzyzuje najszybsze z najwolniejszymi
        {
            do {
                j = rand() % numOfCrossingPairs;
            } while (machineVect[j].isUsed);
            do {
                k = rand() % numOfCrossingPairs + (machineNumber - numOfCrossingPairs);
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

        //        if (prev_value != bestIndivuals[9].fitness) // wyswietli sie tylko wtedy gdy nastapia jakies zmiany
        //        {
        //            cout << "Najsz:  " << bestIndivuals[0].fitness
        //                    << " avg:  " << bestIndivuals[9].fitness // srodkowy
        //                    << " Najw:  " << bestIndivuals[19].fitness // roznica
        //                    << " DIFF:   " << abs(bestIndivuals[0].fitness - bestIndivuals[19].fitness)
        //                    << " Iter:   " << i
        //                    << endl;
        //        }
        prev_value = bestIndivuals[19].fitness;
    }

    //cout << endl;
    //PrintSolution(bestIndivuals);
    return bestIndivuals;
}

/*
vector<Machine> PrepareSchedule() {
    //    vector<Machine> machineVect(machineNumber);
    vector<Machine> tmpVect(machineNumber);

    //vector <vector <double>> matrixETC(machineNumber, vector<double>(taskNumber));
    //vector <double> vectETC(taskNumber);
    vector<Machine> bestIndivuals(machineNumber);

    //ETCGenerator(taskNumber, machineNumber);										To jest wazne - gdy to odkomentujemy to stworzy sie nowa macierz ETC sluzaca dotestow,lepiej nie odkomentowywac bo juz zostala stworzona

    int j = 0;
    int k = 0;
    int point;
//#ifdef ScreenLog
    double prev_value = 0; // uzywany do zapamietywania wartosci poprzedniej
//#endif

    //LoadVectETC(vectETC);


    double actTime = 0.0;
    //int bestIter = 0;
    double bestTime = DBL_MAX;
    vector<Machine> machineVect(machineNumber);
    for (int island = 0; island < maxIslands; island++) {
        machineVect.clear();
        bestIndivuals.clear();
        actTime = 0.0;
        bestTime = DBL_MAX;
        j=0;
        k=0;
        //Init(vectETC, machineVect);
        Init(machineVect);
        //cout << "Island: " << island << endl;
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
//#ifdef ScreenLog
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
//#endif


        }
        // pobierz makespan
        log_makespans.push_back(bestIndivuals[19].fitness);
    }
#ifdef ScreenLog
    cout << endl;
    PrintSolution(bestIndivuals);
#endif

    return bestIndivuals;
}
 */
void PrepareSecureSchedule(int maxIter, int numOfCrossingPairs, int iteration) {
    vector<Machine> machineVect(machineNumber);
    vector<Machine> bestIndivuals(machineNumber);


    int j = 0;
    int k = 0;
    int point;

    double avgTime = 0.0;
    double actTime = 0.0;
    int bestIter = 0;
    double bestTime = DBL_MAX;
    Init(machineVect);
    for (int i = 0; i < maxIter; i++) //pętla główna programu, w niej się wykonują krzyżowania, sortowania itd
    {
        actTime = CountFitness(machineVect, matrixETC);
        if (actTime < bestTime) {
            for (int j = 0; j < machineNumber; j++)
                bestIndivuals[j] = machineVect[j];

            bestIter = i;
            bestTime = actTime;
        }
        //SELECKCJA KRZYŻOWANIE ELITARYZM
        sort(machineVect.begin(), machineVect.end(), Match);

        for (int i = 0; i < machineNumber; i++) //wyzerowanie flagi uzycia
            machineVect[i].isUsed = 0;

        //LOG
        for (std::vector<Machine>::const_iterator i = machineVect.begin(); i != machineVect.end(); ++i)
            avgTime += i->fitness;

        log_securestudy[iteration][i].avgTime = (avgTime / machineVect.size());
        log_securestudy[iteration][i].makespan = machineVect[machineVect.size() - 1].fitness;
        avgTime = 0.0;
        //koniec LOG


        for (int i = machineNumber - numOfCrossingPairs; i < machineNumber; i++) // krzyzuje najszybsze z najwolniejszymi
        {
            do {
                j = rand() % numOfCrossingPairs;
            } while (machineVect[j].isUsed);
            do {
                k = rand() % numOfCrossingPairs + (machineNumber - numOfCrossingPairs);
            } while (machineVect[k].isUsed);

            point = rand() % taskPerMachine;
            Crossing(j, k, point, machineVect);
            machineVect[j].isUsed = 1; // ustawienie flagi aby nie brano już tych maszyn
            machineVect[k].isUsed = 1;

        }

        sort(bestIndivuals.begin(), bestIndivuals.end(), Match); // powinno isc do petli kopiujacej 
        log_securestudy[iteration][i].bestMakespan = bestIndivuals[bestIndivuals.size() - 1].fitness; // a to do logow

    }
}

void exportSecureStudyToCSV(int maxIter, int numOfCrossingPairs, double securityFactor, int repeats) {
    static int i;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char name[50];
    sprintf(name, "logs/Log_SecureStudy_%d-%d-%d_%d.%d.%d_%d.csv", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, i);
    i++;
    ofstream fsout;
    double minMakespan = DBL_MAX;
    double avgMakespan = 0.0;
    fsout.open(name, ios::out);
    if (!fsout.is_open()) {
        cerr << "Blad zapisu do pliku!" << endl;
        return;
    }
    fsout << "Wplyw secure factor na proces genetyczny schedulera\n\n";
    fsout << "Parametry\n";
    fsout << "Powtorzen:;" << repeats << "\n";
    fsout << "Epok:;" << maxIter << "\n";
    fsout << "Liczba par krzyzowanych:;" << numOfCrossingPairs << "\n";
    fsout << "Security Factor:;" << securityFactor << "\n\n";
    for (int i = 0; i < repeats; i++) {
        fsout << "Powtorzenie (iteracja) " << i << "\n";
        fsout << "Epoka;czas sredni kazdego z harmonogramow;makespan;best makespan\n";
        //fsout << "Epoka;czas sredni kazdego z harmonogramow;makespan\n";
        for (int j = 0; j < maxIter; j++) {
            fsout << j << ';' << log_securestudy[i][j].avgTime << ';' << log_securestudy[i][j].makespan << ';' << log_securestudy[i][j].bestMakespan << "\n";
            //fsout << j << ';' << log_securestudy[i][j].avgTime << ';' << log_securestudy[i][j].makespan << "\n";
            avgMakespan += log_securestudy[i][j].makespan;
            if (minMakespan > log_securestudy[i][j].makespan) {
                minMakespan = log_securestudy[i][j].makespan;
            }
        }
        fsout << "Min (best) makespan: " << minMakespan << "; avg makespan: " << (avgMakespan / maxIter) << "\n\n";
        minMakespan = DBL_MAX;
        avgMakespan = 0.0;
    }
    fsout << "KONIEC\n";
    fsout.flush();
    fsout.close();
}

double SL_PrepareSchedule(int maxIter, int numOfCrossingPairs, int securityLevel) {

    vector<Machine> machineVect(machineNumber / securityLevels);
    vector<Machine> bestIndivuals(machineNumber / securityLevels);

    int j = 0;
    int k = 0;
    int point;

    double actTime = 0.0;
    int bestIter = 0;
    double bestTime = DBL_MAX;

    SL_Init(machineVect, securityLevel);

    for (int i = 0; i < maxIter; i++) //pętla główna programu, w niej się wykonują krzyżowania, sortowania itd
    {
        actTime = SL_CountFitness(machineVect, matrixETC);
        if (actTime < bestTime) {
            for (int j = 0; j < machineNumber / securityLevels; j++)
                bestIndivuals[j] = machineVect[j];

            bestIter = i;
            bestTime = actTime;
        }
        //SELECKCJA KRZYŻOWANIE ELITARYZM
        sort(machineVect.begin(), machineVect.end(), Match);

        for (int i = 0; i < machineNumber / securityLevels; i++) //wyzerowanie flagi uzycia
            machineVect[i].isUsed = 0;

        for (int i = machineNumber / securityLevels - numOfCrossingPairs; i < machineNumber / securityLevels; i++) // krzyzuje najszybsze z najwolniejszymi
        {
            do {
                j = rand() % numOfCrossingPairs;
            } while (machineVect[j].isUsed);
            do {
                k = rand() % numOfCrossingPairs + (machineNumber / securityLevels - numOfCrossingPairs);
            } while (machineVect[k].isUsed);

            point = rand() % taskPerMachine;
            Crossing(j, k, point, machineVect);
            machineVect[j].isUsed = 1; // ustawienie flagi aby nie brano już tych maszyn
            machineVect[k].isUsed = 1;

        }
        sort(bestIndivuals.begin(), bestIndivuals.end(), Match);
    }
    return bestIndivuals[(machineNumber / securityLevels) - 1].fitness;
}

double UNI_PrepareSchedule(int maxIter, int numOfCrossingPairs) {

    vector<Machine> machineVect(machineNumber);
    vector<Machine> bestIndivuals(machineNumber);

    int j = 0;
    int k = 0;
    int point;

    double actTime = 0.0;
    int bestIter = 0;
    double bestTime = DBL_MAX;

    Init(machineVect);
    for (int i = 0; i < maxIter; i++) //pętla główna programu, w niej się wykonują krzyżowania, sortowania itd
    {
        actTime = CountFitness(machineVect, matrixETC);
        if (actTime < bestTime) {
            for (int j = 0; j < machineNumber; j++)
                bestIndivuals[j] = machineVect[j];

            bestIter = i;
            bestTime = actTime;
        }
        //SELECKCJA KRZYŻOWANIE ELITARYZM
        sort(machineVect.begin(), machineVect.end(), Match);

        for (int i = 0; i < machineNumber; i++) //wyzerowanie flagi uzycia
            machineVect[i].isUsed = 0;


        for (int i = machineNumber - numOfCrossingPairs; i < machineNumber; i++) // krzyzuje najszybsze z najwolniejszymi
        {
            do {
                j = rand() % numOfCrossingPairs;
            } while (machineVect[j].isUsed);
            do {
                k = rand() % numOfCrossingPairs + (machineNumber - numOfCrossingPairs);
            } while (machineVect[k].isUsed);

            point = rand() % taskPerMachine;
            Crossing(j, k, point, machineVect);
            machineVect[j].isUsed = 1; // ustawienie flagi aby nie brano już tych maszyn
            machineVect[k].isUsed = 1;

        }
        sort(bestIndivuals.begin(), bestIndivuals.end(), Match);


    }

    return bestIndivuals[machineNumber - 1].fitness;
}

void SL_exportToCSV(vector<double> *SL_makespans, vector<double> *UNI_makespans, int epochs, int numOfCrossingPairs, int securityLevels, int repeats) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char name[50];
    sprintf(name, "logs/Log_SecureLevelsStudy_%d-%d-%d_%d.%d.%d.csv", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    ofstream fsout;
    fsout.open(name, ios::out);
    if (!fsout.is_open()) {
        cerr << "Blad zapisu do pliku!" << endl;
        return;
    }
    fsout << "Secure levels w procesie genetycznym schedulera\n\n";
    fsout << "Parametry\n";
    fsout << "Powtorzen:;" << repeats << "\n";
    fsout << "Epok:;" << epochs << "\n";
    fsout << "Liczba par krzyzowanych:;" << numOfCrossingPairs << "\n";
    fsout << "Liczba security levels:;" << securityLevels << "\n\n";

    double SL_sumMakespan = 0.0, UNI_sumMakespan = 0.0;

    fsout << "Powtorzenie;best makespan security level; best makespan universal\n";

    for (int i; i < repeats; ++i) {
        fsout << i << ";" << (*SL_makespans)[i] << ";" << (*UNI_makespans)[i] << '\n';
        SL_sumMakespan += (*SL_makespans)[i];
        UNI_sumMakespan += (*UNI_makespans)[i];
    }
    fsout << "Sredni best makespan;" << SL_sumMakespan / repeats << ";" << UNI_sumMakespan / repeats << "\n";
    fsout << "KONIEC\n";
    fsout.flush();
    fsout.close();
}