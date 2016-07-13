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

#include"Machine.h"

vector<Machine> PrepareSchedule();
double GetTaskTime(int idMachine, int idTask);

#endif /* GASCHED_H */

