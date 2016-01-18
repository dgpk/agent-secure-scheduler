#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <time.h>
#include <openssl/ssl.h> 
#include <math.h>
#include <openssl/rand.h>
#include <openssl/bn.h>
#include "global_fun.h"

#define ulong unsigned long int

using namespace std;

double matrix_multiplication(size_t size, size_t ntimes)
{
    int i, j, k, l;
    clock_t t1, t2;
    t1 = clock();
   

    srand(time(NULL));

    double **matrix;
    double **result;
    matrix = (double**)malloc(size * sizeof(double*));
    result = (double**)malloc(size * sizeof(double*));
    for(i = 0; i < size; i++) {
        matrix[i] = (double*)malloc(size * sizeof(double));
        result[i] = (double*)malloc(size * sizeof(double));
    }

    double sum=0;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            matrix[i][j] = (double)(rand()) / (double)(RAND_MAX);
        }
    }

    for(l = 0; l < ntimes; l++)
    {
        for (i = 0; i < size; i++) {
            for (j = 0; j < size; j++) {
                for (k = 0; k < size; k++) {
                    sum = sum + matrix[i][k]*matrix[k][j];
                }
                result[i][j] = sum;
                sum = 0.0f;
            }
        }
    }
    for(i = 0; i < size; i++) {
        free(matrix[i]);
        free(result[i]);
    }
    free(matrix);
    free(result);
    matrix = NULL;
    result = NULL;
   
    t2 = clock();
    return (((double)t2 - (double)t1) / 1000000.0D ) * 1000;  
    //printf("Execution time: %f ms\n",diff);
}


void get_SHA512(char* str, char* res)
{
    unsigned char digest[SHA512_DIGEST_LENGTH];
    //char str[] = "Seed";
    SHA512(reinterpret_cast<const unsigned char*>(str), strlen(str), (unsigned char*)&digest);
    //SHA512((unsigned char*)&str, strlen(str), (unsigned char*)&digest);
 
    /*char mdString[SHA512_DIGEST_LENGTH*2+1];
 
    for(int i = 0; i < SHA512_DIGEST_LENGTH; i++)
         sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
 
    printf("SHA512 digest: %s\n", mdString);
    */
    char *res_tmp;
    res_tmp = res;
    for(int i = 0; i < SHA512_DIGEST_LENGTH; i++)
    {
        res_tmp+=sprintf(res_tmp, "%02x", (unsigned int)digest[i]);
    }
    //sprintf(res, "%02x", digest);
    //printf("SHA512 digest: %s\n", res);
    //return mdString;
}

ulong* generateBBSKey(ulong N) {
	BIGNUM *p, *q, *n, *nn, *s, *TWO, **x, *tmpZ;
	BN_CTX *bnCtx;
	ulong *z;
	static const char rnd_seed[] =
			"string to make the random number generator think it has entropy";

	x = (BIGNUM**) malloc((N + 1) * sizeof(BIGNUM*));
	for (ulong i = 0; i < N + 1; i++) {
		x[i] = BN_new();
	}
	z = (ulong*) malloc((N) * sizeof(ulong));
	p = BN_new();
	q = BN_new();
	n = BN_new();
	nn = BN_new();
	s = BN_new();
	tmpZ = BN_new();
	TWO = BN_new();
	bnCtx = BN_CTX_new();
	RAND_seed(rnd_seed, sizeof rnd_seed); /* or BN_generate_prime_ex may fail */

	BN_generate_prime_ex(p, 768, 0, NULL, NULL, NULL);
	BN_generate_prime_ex(q, 768, 0, NULL, NULL, NULL);
	BN_mul(n, p, q, bnCtx);
	BN_set_word(TWO, 2);
	BN_sub(nn, n, TWO);
	BN_rand_range(s, nn);
	BN_add(s, s, BN_value_one());

	BN_mod_mul(x[0], s, s, n, bnCtx);
       

	for (ulong i = 1; i < N + 1; i++) {
		BN_mod_mul(x[i], x[i - 1], x[i - 1], n, bnCtx);
		BN_mod(tmpZ, x[i], TWO, bnCtx);
		z[i-1] = BN_get_word(tmpZ); // bylo z[i]
	}
	BN_free(p);
	BN_free(q);
	BN_free(n);
	BN_free(nn);
	BN_free(s);
	BN_free(tmpZ);
	BN_free(TWO);
	BN_CTX_free(bnCtx);
	return z;
}