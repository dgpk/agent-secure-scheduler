/* 
 * File:   global_fun.h
 * Author: daniel
 *
 * Created on 4 styczeń 2016, 22:04
 */

#ifndef GLOBAL_FUN_H
#define GLOBAL_FUN_H

#define ulong unsigned long int

double matrix_multiplication(size_t size, size_t ntimes);

//char* get_SHA512(char *str);
void get_SHA512(char* str, char* res);

ulong* generateBBSKey(ulong N);

#endif	/* GLOBAL_FUN_H */

