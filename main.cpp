/* 
 * File:   main.cpp
 * Author: daniel
 *
 * Created on 23 grudzień 2015, 13:50
 */


#include <cstdlib>
#include <iostream>
#include <vector>
#include <time.h>
#include <openssl/ssl.h> 
#include <ff/farm.hpp>
#include <ff/pipeline.hpp>
#include "global_fun.h"

using namespace std;
using namespace ff;

#define ulong unsigned long int

struct WorkerA: ff_node_t<long> {
  long *svc(long *task) { 
          
      std::cout << "WorkerA has got the task " << *task << "\n";
      /* if  *task % 5==0 licz zadanie małe 
       * if  *task % 5==1 licz zadanie srednie
       * if  *task % 5 =2 licz zadanie duze
       * if  * task % 5 =3  licz zadanie bardzoduze
       * if  *task  % 5 =4 licz zadanie olbrzymie
       
       */
      if(*task %5 == 0)
          //double exec_time = matrix_multiplication(128, 5);
          // pierwszy argument to wymiar macierzy kwadratowej, tutaj 128x128
          // drugi, to ile razy ma się to przemnożyć
       
      return task;
  }
};


struct WorkerB: ff_node_t<long> {
  long *svc(long *task) { 
      std::cout << "WorkerB has got the task " << *task << "\n";
        /* if  *task % 5==0 licz zadanie małe 
       * if  *task % 5==1 licz zadanie srednie
       * if  *task % 5 =2 licz zadanie duze
       * if  * task % 5 =3  licz zadanie bardzoduze
       * if  *task  % 5 =4 licz zadanie olbrzymie
       
       */
            return task;
  }
};


struct WorkerC: ff_node_t<long> {
  long *svc(long *task) { 
      std::cout << "WorkerC has got the task " << *task << "\n";
        /* if  *task % 5==0 licz zadanie małe 
       * if  *task % 5==1 licz zadanie srednie
       * if  *task % 5 =2 licz zadanie duze
       * if  * task % 5 =3  licz zadanie bardzoduze
       * if  *task  % 5 =4 licz zadanie olbrzymie
       
       */
      return task;
  }
};


struct WorkerD: ff_node_t<long> {
  long *svc(long *task) { 
      std::cout << "WorkerD has got the task " << *task << "\n";
      
        /* if  *task % 5==0 licz zadanie małe 
       * if  *task % 5==1 licz zadanie srednie
       * if  *task % 5 =2 licz zadanie duze
       * if  * task % 5 =3  licz zadanie bardzoduze
       * if  *task  % 5 =4 licz zadanie olbrzymie
       
       */
      return task;
  }
};

struct Emitter: ff_node_t<long> {
  Emitter(ff_loadbalancer *const lb):lb(lb) {}
  ff_loadbalancer *const lb;
  const long size=100;  
  long *svc(long *) { 
    for(long i=0; i <= size; ++i) {
	lb->broadcast_task(new long(i));
        //lb->broadcast_task((void*)i);
        //cout << i << endl;
        // Co do poniższego - tak, ale zamiast tasków 1, 2, 3 i 4 są jakies głupoty... :/
        // Edit: Głupoty tylko w wersji DEBUG, w RELEASE nie zauważyłem
        
        
        /*rozrzuca taski po workerach  mamy 100 liczb do rozrzucenia 
         powiedzmy, ze na razie zrobimy tak: do workerów trafiają liczby: 1,2... lub 100. 
         * powiedzmy ze mamy 5 typów zadan: małe, srednie,duze, bardzo duze, olbrzymie 
         * 
         * jesli do workera trafi liczba   i (mond 5)=0 worker liczy zadanie małe
         * jesli do workera trafi liczba   i (mond 5)=1 worker liczy zadanie srednie
         * jesli do workera trafi liczba   i (mond 5)=2 worker liczy zadanie duze
         * jesli do workera trafi liczba   i (mond 5)=3 worker liczy zadanie bardzoduze
         * jesli do workera trafi liczba   i (mond 5)=4 worker liczy zadanie olbrzymie
         * 
         * 
         * 
         
         */
    }
    return EOS;
  }
};
struct Collector: ff_node_t<long> {
  Collector(ff_gatherer *const gt):gt(gt) {}
  ff_gatherer *const gt;
  long *svc(long *task) { 
      std::cout << "received task from Worker " << gt->get_channel_id() << "\n";
      if (gt->get_channel_id() == 0) delete task;
      return GO_ON;
  }
};

// nasze zadanie testowe - niezrównoleglone


int main(int argc, char **argv) {
  
  int nworkers = 4;

  std::vector<std::unique_ptr<ff_node> > Workers;

  
  for(int i=0;i<nworkers/2;++i)        Workers.push_back(make_unique<WorkerA>());
  for(int i=nworkers/2;i<nworkers;++i) Workers.push_back(make_unique<WorkerB>());
  
  ff_Farm<> farm(std::move(Workers)); 
  Emitter E(farm.getlb());
  Collector C(farm.getgt());
  farm.add_emitter(E);    // add the specialized emitter
  farm.add_collector(C);  
  if (farm.run_and_wait_end()<0) error("running farm"); 
  
    cout << "Press enter to continue ..." << endl;
    cin.get();
    
    // Mnożenie macierzy
    double exec_time = 0.0;
    // macierz 128x128 pięć razy liczona
    exec_time = matrix_multiplication(128, 5);
    printf("Execution time: %lf ms\n",exec_time);
    
    //SHA512
try
{
    char str[] = "Seed"; // wiadomosc
    char *mdString = NULL; // skrot
    mdString = new char[SHA512_DIGEST_LENGTH*2+1];
    get_SHA512(str, mdString); // przekazujemy wiadomosc i zaalokowane miejsce na skrot
        printf("SHA512 digest: %s\n", mdString);
    delete[] mdString;
}
catch (bad_alloc)
{
    cerr << "Blad alokacji (SHA)" << endl;
}
cout << endl;
    
    //BBS
ulong *key;
ulong N;
N = 100;
key = generateBBSKey(N );
for(ulong i=0;i<N;i++){
    std::cout << key[i];
}
free(key);


cout << endl;

 return 0;
}



