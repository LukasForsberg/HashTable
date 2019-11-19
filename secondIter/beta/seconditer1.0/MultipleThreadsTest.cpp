#include "HashTable.h"
#include <iostream>
#include <string>
#include <cassert>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

  int *randTable = new int[1000];

  HashTable<int,int> writeTable = HashTable<int,int>(128);

  HashTable<int,int> reHashTable = HashTable<int,int>(128);

  HashTable<int,int> spamTable = HashTable<int,int>(128);



//---------------------------------HELP_FUNCTIONS----------------------------//

void *write(void *arg){
  for(int j =  0; j < 10; j++){
    writeTable.singleWrite(j, randTable[j]);
  }
}

void *hashWrite(void *arg){
  int index = *((int*)(&arg));
  for(int j =  100*(int )index; j < 100*((int) index+1); j++){
    reHashTable.singleWrite(j, randTable[j]);
  }
}

void *spamWrite(void *arg){
  for(int i = 0; i < 10; i++){
    spamTable.singleWrite(1, spamTable.singleRead(1) + 1);
  }
}

//----------------------------------TESTS-------------------------------------//

void writeAndReadTest(){

  cout << "writeAndRead: RUNNING..." << endl;

  int no_threads = 5;

  pthread_t *threads = new pthread_t[no_threads];

  for( int i = 0; i < no_threads*10; i++ ) {
    randTable[i] = rand() % 100;
  }

  for( int i = 0; i < no_threads; i++ ) {

    pthread_create(&threads[i], NULL, &write,(void*)i);
  }

  for (int i = 0; i < no_threads; i++)
    {
       pthread_join (threads[i], NULL);
    }

  for( int i = 0; i < 10; i++) {

    cout << writeTable.singleRead(i) << endl;
    assert(writeTable.singleRead(i) == randTable[i]);


  }

  cout << "writeAndReadTest: OK" << endl;


}

//PASSES SOMETIMES?? --> NOT THREADSAFE
void reHashTest(){

  cout << "reHashTest: RUNNING..." << endl;

  int no_threads = 5;

  pthread_t *threads = new pthread_t[no_threads];

  for( int i = 0; i < no_threads*100; i++ ) {

    randTable[i] = rand() % 100;
  }

  for( int i = 0; i < no_threads; i++ ) {

    pthread_create(&threads[i], NULL, &hashWrite,(void*)i);
  }

  for (int i = 0; i < no_threads; i++)
    {
       pthread_join (threads[i], NULL);
    }

  for( int i = 0; i < no_threads*100; i++) {

    cout << i << endl;
    assert(reHashTable.singleRead(i) == randTable[i]);

  }

  cout << "rehashTest: OK" << endl;

}

//NEVER PASSES (INVALID READ EXCEPTION)
void spamBucketTest(){

  cout << "spamBucketTest: RUNNING..." << endl;


  int no_threads = 10;
  spamTable.singleWrite(1, 0);
  pthread_t *threads = new pthread_t[no_threads];

  for( int i = 0; i < no_threads; i++ ) {

    pthread_create(&threads[i], NULL, &spamWrite, (void*)i);
  }

  for (int i = 0; i < no_threads; i++)
    {
       pthread_join (threads[i], NULL);
    }


  assert(spamTable.singleRead(1) == 10*no_threads);

  cout << "spamBucketTest: OK" << endl;

}


//-----------------------------------MAIN-------------------------------------//

int main(){

  //writeAndReadTest();
  reHashTest();
  //spamBucketTest();


}
