#include <iostream>
#include "Arena.h"
#include "HashNode.h"

using namespace std;

int main(){

  struct timespec start, end;
  HashNode<int,int> *p1[1000];
  HashNode<int,int> *p2[1000];

  clock_gettime(CLOCK_REALTIME, &start);
  Arena<int,int> arena(1000);

  for(size_t i = 0; i < 1000; i++){
    p1[i] = arena.alloc(i,i+1);
  }
  for(size_t i = 0; i < 1000; i++){
    arena.free(p1[i]);
  }
  for(size_t i = 0; i < 1000; i++){
    p1[i] = arena.alloc(i,i+1);
  }
  clock_gettime(CLOCK_REALTIME, &end);
  auto sum1 = end.tv_nsec - start.tv_nsec;
  cout << "time 1: " << sum1 << endl;


  clock_gettime(CLOCK_REALTIME, &start);

  for(size_t i = 0; i < 1000; i++){
    p2[i] = new HashNode<int,int>(i,i+1);
  }
  for(size_t i = 0; i < 1000; i++){
    free(p2[i]);
  }
  for(size_t i = 0; i < 1000; i++){
    p2[i] = new HashNode<int,int>(i,i+1);
  }
  clock_gettime(CLOCK_REALTIME, &end);
  auto sum2 = end.tv_nsec - start.tv_nsec;
  cout << "time 2: " << sum2 << endl;



/*
  for(size_t i = 0; i < 1000; i++){
    cout  << p1[i]->getKey() << " " << p1[i]->getValue() << " ";
    cout  << p2[i]->getKey() << " " << p2[i]->getValue() << " " ;
  }
*/

//  arena.free(p1);
//  arena.free(p2);

  return 0;

}
