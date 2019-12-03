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

  int *randTable = new int[10000];

  HashTable<int,int> writeTable = HashTable<int,int>(128);

  HashTable<int,int> reHashTable = HashTable<int,int>(128);

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
  for(int j =  100*(int )index; j < 100*((int) index+1); j++){
    reHashTable.singleWrite(j, randTable[j]);
  }
  return arg;
}

void* spamWrite(void *arg){
  int index = (*(int*)arg);
  for(int i = 0; i < 100; i++){
    spamTable.singleWrite(4096*( i+ index*100), 4096*( i+ index*100));
  }
  return arg;
}

typedef struct mega_data{
  HashTable<int,int>* table;
  vector<pair<int,int>> vec;
} mega_data;

void *megaWrite(void *arg){
  mega_data* data = (mega_data*)arg;
  int key;
  int value;
  for(int i = 0; i < 1000; i++){
    value = rand();
    do {
      key = rand();
    }while(data->table->contains(key));
    data->table->singleWrite(key, value);
    data->vec.push_back(make_pair(key,value));
  }
  return arg;
}

void *megaRead(void *arg){
  HashTable<int,int>* table = (HashTable<int,int>*)arg;
  int key;
  for(int i = 0; i < 1000; i++){
    key = rand();
    try{
      table->singleRead(key);
    } catch(InvalidReadExeption& e){}
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

  for (int i = 0; i < no_threads; i++){
       pthread_join (threads[i], NULL);
  }

  for( int i = 0; i < 10; i++) {
    assert(writeTable.singleRead(i) == randTable[i]);
  }
  delete [] threads;
  cout << "writeAndReadTest: OK" << endl;
}

void reHashTest(){

  cout << "reHashTest: RUNNING..." << endl;
  int no_threads = 10;
  pthread_t *threads = new pthread_t[no_threads];

  for( int i = 0; i < no_threads*100; i++ ) {
    randTable[i] = rand() % 100;
  }

  for( int i = 0; i < no_threads; i++ ) {
    pthread_create(&threads[i], NULL, &hashWrite,(void*)i);
  }

  for (int i = 0; i < no_threads; i++){
    pthread_join (threads[i], NULL);
  }

  for( int i = 0; i < no_threads*100; i++) {
    assert(reHashTable.singleRead(i) == randTable[i]);
  }
  delete [] threads;
  cout << "rehashTest: OK" << endl;
}

void spamBucketTest(){

  cout << "spamBucketTest: RUNNING..." << endl;

  int no_threads = 10;
  pthread_t *threads = new pthread_t[no_threads];
  int* index = new int[10];

  for(int i = 0; i < no_threads; i++ ) {
    index[i] = i;
    pthread_create(&threads[i], NULL, &spamWrite, (void*)&index[i]);
  }

  for (int i = 0; i < no_threads; i++){
       pthread_join (threads[i], NULL);
  }

  for(int i = 0; i < 100; i++){
    assert(spamTable.singleRead(i*4096) == i*4096);
  }
  delete [] threads;
  delete [] index;
  cout << "spamBucketTest: OK" << endl;
}

void megaSpamTest(){
  cout << "megaSpamTest: RUNNING..." << endl;
  int no_threads = 10;
  int no_write_threads = no_threads/2;
  int no_read_threads = no_threads/2;
  pthread_t *threads = new pthread_t[no_threads];
  mega_data* thread_data = new mega_data[no_write_threads];
  HashTable<int,int> table = HashTable<int,int>(128);

  for(int i = 0; i < no_write_threads; i++){
    thread_data[i].table = &table;
  }

  for( int i = 0; i < no_write_threads; i++ ) {
    pthread_create(&threads[i], NULL, &megaWrite, (void*)&thread_data[i]);
    pthread_create(&threads[i + no_read_threads], NULL, &megaRead, (void*)&table);
  }

  for (int i = 0; i < no_threads; i++){
      pthread_join (threads[i], NULL);
  }

  int count = 0;
  for(int i = 0; i < no_write_threads; i++){
    for(auto p : thread_data[i].vec){
      try{
        if(table.singleRead(p.first) != p.second){
          for(int j = 0; j < no_write_threads; j++){
            for(auto p2 : thread_data[j].vec){
              if(p.first == p2.first){
                // if two keys is by chance equal
                count++;
              }
            }
          }
        }
      } catch(InvalidReadExeption& e){
        cout << "failed to read key " << p.first << endl;
        assert(false);
      }
      if(count > 1){
        count = 0;
        continue;
      }
    assert(table.singleRead(p.first) == p.second);
    }
  }
  delete [] threads;
  delete [] thread_data;
  cout << "megaSpamTest: OK" << endl;
}
/*
void writeReadDeleteTest(){
  cout << "writeReadDeleteTest: RUNNING..." << endl;
  int no_threads = 12;
  int no_write_threads = no_threads/2;
  int no_read_threads = no_threads/3;
  int no_delete_threads = no_threads/6;
  pthread_t *w_threads = new pthread_t[no_write_threads];
  pthread_t *r_threads = new pthread_t[no_read_threads];
  pthread_t *d_threads = new pthread_t[no_delete_threads];

  mega_data* thread_data = new mega_data[no_write_threads + no_delete_threads];
  HashTable<int,int> table = HashTable<int,int>(128);

  for(int i = 0; i < no_write_threads + no_delete_threads; i++){
    thread_data[i].table = &table;
  }

  for( int i = 0; i < no_write_threads; i++ ) {
    pthread_create(&threads[i], NULL, &megaWrite, (void*)&thread_data[i]);
  }
  for( int i = 0; i < no_write_threads; i++ ) {
    pthread_create(&threads[i + no_read_threads], NULL, &megaRead, (void*)&table);
  }
  for( int i = 0; i < no_write_threads; i++ ) {
    pthread_create(&threads[i + no_read_threads], NULL, &megaRead, (void*)&table);
  }


  for (int i = 0; i < no_threads; i++){
      pthread_join (threads[i], NULL);
  }

  delete [] w_threads;
  delete [] r_threads;
  delete [] d_threads;
  delete [] thread_data;
}
*/

//-----------------------------------MAIN-------------------------------------//

int main(){

  writeAndReadTest();
  reHashTest();
  spamBucketTest();
  megaSpamTest();
  delete [] randTable;

}
