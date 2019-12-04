#include "HashTable.h"
#include <iostream>
#include <string>
#include <cassert>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <vector>
#include <utility>

using std::vector;
using std::pair;

  int *randTable = new int[1000000];

  HashTable<int,int> writeTable = HashTable<int,int>(128);

  HashTable<int,int> reHashTable = HashTable<int,int>(16777216);

  HashTable<int,int> spamTable = HashTable<int,int>(256);


//---------------------------------HELP_FUNCTIONS----------------------------//

void* write(void *arg){
  for(int j =  0; j < 10; j++){
    writeTable.singleWrite(j, randTable[j]);
  }
  return arg;
}

void* hashWrite(void *arg){
  int index = *((int*)(&arg));
  for(int j =  1000*(int )index; j < 1000*((int) index+1); j++){
    reHashTable.singleWrite(j, randTable[j]);
  }
  return arg;
}

void* spamWrite(void *arg){
  for(int i = 0; i < 10; i++){
    spamTable.singleWrite(1, spamTable.singleRead(1) + 1);

  }
  return arg;
}

typedef struct mega_data{
  HashTable<int,int>* table;
  vector<pair<int,int>> vec;
} mega_data;

void *megaWrite(void *arg){
  mega_data* data = (mega_data*)arg;
  for(int i = 0; i < 1000; i++){
    int key;
    do {
      key = rand();
    }while(data->table->contains(key));
    int value = rand();
    data->table->singleWrite(key, value);
    data->vec.push_back(make_pair(key,value));
  }
  return arg;
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

    pthread_create(&threads[i], NULL, &write,NULL);
  }

  for (int i = 0; i < no_threads; i++)
    {
       pthread_join (threads[i], NULL);
    }

  for( int i = 0; i < 10; i++) {

    assert(writeTable.singleRead(i) == randTable[i]);


  }

  cout << "writeAndReadTest: OK" << endl;


}

  void reHashTest(){

  reHashTable.hashSum.tv_nsec = 0;
  reHashTable.hashSum.tv_sec = 0;

  srand (time(0));

  for( int i = 0; i < 1000000; i++ ) {

    randTable[i] = rand() % 100;
  }

  for( int i = 0; i < 1000000; i++ ) {
    reHashTable.singleWrite(i, randTable[i]);
  }

  reHashTable.rehash();

  for( int i = 0; i < 1000000; i++) {

    assert(reHashTable.singleRead(i) == randTable[i]);

  }


  cout << "Rehash time: "  << reHashTable.nanoTotal << endl;
  cout << "rehashTest: OK" << endl;

}

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

void megaSpamTest(){
  cout << "megaSpamTest: RUNNING..." << endl;
  int no_threads = 10;
  pthread_t *threads = new pthread_t[no_threads];
  mega_data* thread_data = new mega_data[no_threads];
  HashTable<int,int> table = HashTable<int,int>(128);

  for(int i = 0; i < no_threads; i++){
    thread_data[i].table = &table;
  }

  for( int i = 0; i < no_threads; i++ ) {
    pthread_create(&threads[i], NULL, &megaWrite, (void*)&thread_data[i]);
  }

  for (int i = 0; i < no_threads; i++){
       pthread_join (threads[i], NULL);
  }

  for(int i = 0; i < no_threads; i++){
    for(auto p : thread_data[i].vec){
      assert(table.singleRead(p.first) == p.second);
    }
  }
  cout << "megaSpamTest: OK" << endl;
}


//-----------------------------------MAIN-------------------------------------//

int main(){

  //writeAndReadTest();
  /*uint64_t sum = 0;
  uint64_t part_sum;
  for (int i = 0; i < 10000; i++){
   part_sum = reHashTest();
   if(part_sum < 104467440727924209){
     sum = sum + part_sum;
   }
  }
  cout << sum / 100 << endl;
  */
  reHashTest();
  //spamBucketTest();
  //megaSpamTest();


}
