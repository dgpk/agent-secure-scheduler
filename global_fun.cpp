//#include <cstdlib>
//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <cstdio>
//#include <fstream>
//#include <iostream>
//#include <time.h>
//#include <openssl/ssl.h> 
//#include <math.h>

#include "global_fun.h"


#define ulong unsigned long int

using namespace std;

void init_logs(int numOfPackages) {
    if (log_batch)
        delete log_batch;
    log_batch = new BatchLog;
    log_tasks.clear();
    log_packages.clear();
    for (int i = 0; i < numOfPackages; i++) {
        log_packages.push_back(PackageLog(i, 0, 0));
    }
    log_schedule.clear();
}

void exportToCSV(char *model, unsigned int processingTime, unsigned int numOfPackages, unsigned int sizeOfPackage) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char name[50];
    sprintf(name, "logs/Log_%s_%d-%d-%d_%d.%d.%d.csv", model, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    ofstream fsout;
    fsout.open(name, ios::out);
    if (!fsout.is_open()) {
        cerr << "Blad zapisu do pliku!" << endl;
        return;
    }
    fsout << model << "\n\n";
    fsout << "Parametry\n";
    fsout << "processingTime:;" << processingTime << "\n";
    fsout << "numOfPackages:;" << numOfPackages << "\n";
    fsout << "sizeOfPackage:;" << sizeOfPackage << "\n\n";

    fsout << "Czas wykonywania poszczegolnych zadan [s]\n";
    fsout << "workerType;workerID;packageID;taskNumber;taskTime\n";
    for (vector<TaskLog>::iterator it = log_tasks.begin(); it != log_tasks.end(); ++it) {
        fsout << it->workerType << ";" << it->workerID << ";" << it->packageID << ";" << it->taskNumber << ";" << double(it->endTime - it->beginTime) / CLOCKS_PER_SEC << "\n";
    }
    fsout << "\n";
    fsout << "Czas wykonywania poszczegolnych pakietow [s]\n";
    fsout << "packageID;packageTime;start;stop\n";
    for (vector<PackageLog>::iterator it = log_packages.begin(); it != log_packages.end(); ++it) {
        fsout << it->packageID << ";" << double(it->endTime - it->beginTime) / CLOCKS_PER_SEC <<";"<< it->beginTime  << ";" << it->endTime << "\n";
    }

    fsout << "\n";
    fsout << "Harmonogramy maszyn dla poszczegolnych pakietow [s]\n";
    fsout << "packageID;czas tworzenia harmonogramu\n";
    for (vector<ScheduleLog>::iterator it = log_schedule.begin(); it != log_schedule.end(); ++it) {
        fsout << it->packageID << ";" << double(it->endTime - it->beginTime) / CLOCKS_PER_SEC << "\n";
        fsout << "Harmonogram:" << "\n";
        for (vector<Machine>::iterator it2 = it->schedule.begin(); it2 != it->schedule.begin(); ++it2) {
            fsout << *it2 << '\n';
        }
    }


    fsout << "\n";
    fsout << "Czas wykonywania calego wsadu [s]\n";
    fsout << double(log_batch->endTime - log_batch->beginTime) / CLOCKS_PER_SEC << "\n\n";
    fsout << "KONIEC\n";
    fsout.flush();
    fsout.close();
}

//Przygotowanie zadania, które wiemy dokładnieile będzie trwało

void czekaj(int iSekundy) {
    for (clock_t koniec = clock() + iSekundy * CLOCKS_PER_SEC; clock() < koniec;)
        continue;

}

// Inicjalizujemy semafory (funkcja wywoływana raz w main)

void init_sem(unsigned int numOfPackages) {
    semTable = new sem_t[numOfPackages];
    for (int i = 0; i < numOfPackages; i++) {
        sem_init(&semTable[i], 0, 1);
        //taskTable[i] = sizeOfPackage;
    }
    //threads = new pthread_t[numOfPackages];
}

// Opuszczamy semafory (funkcja wywoływana w każdym emmiterze na początku)

void down_sem(unsigned int numOfPackages) {
    for (int i = 0; i < numOfPackages; i++) {
        sem_wait(&semTable[i]);
        //taskTable[i] = sizeOfPackage;
    }
}

// Niszczymy semafory (funkcja wywoływana raz w main - na końcu)

void destroy(unsigned int numOfPackages) {
    for (int i = 0; i < numOfPackages; i++)
        sem_destroy(&semTable[i]);
    delete[] semTable;
}

// Funkcja która zarządza dostępem do paczek zadań 
// czyli zadania przychodzą w określonych odstępach czasu
// Wyświetla informacje o paczce i ile musi czekac na przyjscie
// Po tym czasie podnosi semafor, czyli daje dostep do tych danych
// Po podniesieniu wszystkich semaforów wątek ginie

void* thread_timer(void *ptr) {
    TimerData *td = (TimerData *) ptr;
    for (int i = 0; i < td->numOfPackages; i++) {
        cout << "Paczka " << i << "  Watek czeka " << td->timeTable[i] << endl;
        sleep(td->timeTable[i]);
        sem_post(&semTable[i]);
        cout << "podniesiono semafor " << i << endl;
    }
    pthread_exit(0);

}

double matrix_multiplication(size_t size, size_t ntimes) {
    int i, j, k, l;
    clock_t t1, t2;
    t1 = clock();


    srand(time(NULL));

    double **matrix;
    double **result;
    matrix = (double**) malloc(size * sizeof (double*));
    result = (double**) malloc(size * sizeof (double*));
    for (i = 0; i < size; i++) {
        matrix[i] = (double*) malloc(size * sizeof (double));
        result[i] = (double*) malloc(size * sizeof (double));
    }

    double sum = 0;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            matrix[i][j] = (double) (rand()) / (double) (RAND_MAX);
        }
    }

    for (l = 0; l < ntimes; l++) {
        for (i = 0; i < size; i++) {
            for (j = 0; j < size; j++) {
                for (k = 0; k < size; k++) {
                    sum = sum + matrix[i][k] * matrix[k][j];
                }
                result[i][j] = sum;
                sum = 0.0f;
            }
        }
    }
    for (i = 0; i < size; i++) {
        free(matrix[i]);
        free(result[i]);
    }
    free(matrix);
    free(result);
    matrix = NULL;
    result = NULL;

    t2 = clock();
    return (((double) t2 - (double) t1) / 1000000.0D) * 1000;
    //printf("Execution time: %f ms\n",diff);
}

void get_SHA512(char* str, char* res) {
    unsigned char digest[SHA512_DIGEST_LENGTH];
    //char str[] = "Seed";
    SHA512(reinterpret_cast<const unsigned char*> (str), strlen(str), (unsigned char*) &digest);
    //SHA512((unsigned char*)&str, strlen(str), (unsigned char*)&digest);

    /*char mdString[SHA512_DIGEST_LENGTH*2+1];
 
    for(int i = 0; i < SHA512_DIGEST_LENGTH; i++)
         sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
 
    printf("SHA512 digest: %s\n", mdString);
     */
    char *res_tmp;
    res_tmp = res;
    for (int i = 0; i < SHA512_DIGEST_LENGTH; i++) {
        res_tmp += sprintf(res_tmp, "%02x", (unsigned int) digest[i]);
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

    x = (BIGNUM**) malloc((N + 1) * sizeof (BIGNUM*));
    for (ulong i = 0; i < N + 1; i++) {
        x[i] = BN_new();
    }
    z = (ulong*) malloc((N) * sizeof (ulong));
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
        z[i - 1] = BN_get_word(tmpZ); // bylo z[i]
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

void gen_random(char *s, const int len) {
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof (alphanum) - 1)];
    }

    s[len] = 0;
}

const char* hex_char_to_bin(char c) {
    switch (toupper(c)) {
        case '0': return "0000";
            break;
        case '1': return "0001";
            break;
        case '2': return "0010";
            break;
        case '3': return "0011";
            break;
        case '4': return "0100";
            break;
        case '5': return "0101";
            break;
        case '6': return "0110";
            break;
        case '7': return "0111";
            break;
        case '8': return "1000";
            break;
        case '9': return "1001";
            break;
        case 'A': case 'a': return "1010";
            break;
        case 'B': case 'b': return "1011";
            break;
        case 'C': case 'c': return "1100";
            break;
        case 'D': case 'd': return "1101";
            break;
        case 'E': case 'e': return "1110";
            break;
        case 'F': case 'f': return "1111";
            break;
        default:
            cerr << "Error hex to bin" << endl;
            getchar();
            exit(-1);
            break;
    }
}