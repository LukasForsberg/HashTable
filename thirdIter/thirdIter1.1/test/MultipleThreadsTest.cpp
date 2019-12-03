#include "../src/Table/HashTable.h"
#include <iostream>
#include <string>
#include <cassert>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <vector>
#include <utility>
#include <map>

using std::vector;
using std::pair;

  int *randTable = new int[10000];

  HashTable<int,int> writeTable = HashTable<int,int>(128);

  HashTable<int,int> reHashTable = HashTable<int,int>(8192);

  HashTable<int,int> spamTable = HashTable<int,int>(128);

//---------------------------------HELP_FUNCTIONS----------------------------//

void* write(void *arg){
  for(int j =  0; j < 10; j++){
    writeTable.write(j, randTable[j]);
  }
  return arg;
}

void* hashWrite(void *arg){
  int index = *((int*)(&arg));
<<<<<<< HEAD
  for(int j =  1000*(int )index; j < 1000*((int) index+1); j++){
    reHashTable.singleWrite(j, randTable[j]);
=======
  for(int j =  100*(int )index; j < 100*((int) index+1); j++){
    reHashTable.write(j, randTable[j]);
>>>>>>> cf7ff489bdf05b06994f3847a169933c74974a03
  }
  return arg;
}

void* spamWrite(void *arg){
  int index = (*(int*)arg);
  for(int i = 0; i < 100; i++){
    spamTable.write(4096*( i+ index*100), 4096*( i+ index*100));
  }
  return arg;
}

typedef struct mega_data{
  HashTable<int,int>* table;
  map<int,int> my_map;
  map<int,int> deleted;
} mega_data;

void *megaWrite(void *arg){
  mega_data* data = (mega_data*)arg;
  int key;
  int value;
  for(int i = 0; i < 400; i++){
    value = rand();
    do {
      key = rand();
    }while(data->table->contains(key));
    data->table->write(key, value);
    data->my_map.insert(make_pair(key,value));
  }
  return arg;
}

void *megaRead(void *arg){
  HashTable<int,int>* table = (HashTable<int,int>*)arg;
  int key;
  for(int i = 0; i < 100; i++){
    key = rand();
    try{
      table->read(key);
    } catch(InvalidReadExeption& e){}
  }
  return arg;
}

void *mapDelete(void *arg){
  mega_data* data = (mega_data*)arg;
  int i = 0;
  for(auto p : data->my_map){
    if(data->table->remove(p.first)){
      data->deleted.insert(make_pair(p.first, 0));
      i++;
    }
    if(i > 100){
      break;
    }
  }
  return arg;
}

void *mapWrite(void *arg){
  mega_data* data = (mega_data*)arg;
  for(auto p : data->my_map){
    data->table->write(p.first, p.second);
  }
  return arg;
}

void *mapRead(void *arg){
  mega_data* data = (mega_data*)arg;
  for(auto p : data->my_map){
    try{
      data->table->read(p.first);
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
    assert(writeTable.read(i) == randTable[i]);
  }
  delete [] threads;
  cout << "writeAndReadTest: OK" << endl;
}

 uint64_t reHashTest(){

  cout << "reHashTest: RUNNING..." << endl;
  int no_threads = 10;
  pthread_t *threads = new pthread_t[no_threads];

  for( int i = 0; i < no_threads*1000; i++ ) {
    randTable[i] = rand() % 1000;
  }

  for( int i = 0; i < no_threads; i++ ) {
    pthread_create(&threads[i], NULL, &hashWrite,(void*)i);
  }

  for (int i = 0; i < no_threads; i++){
    pthread_join (threads[i], NULL);
  }

<<<<<<< HEAD
  for( int i = 0; i < no_threads*1000; i++) {
    assert(reHashTable.singleRead(i) == randTable[i]);
=======
  for( int i = 0; i < no_threads*100; i++) {
    assert(reHashTable.read(i) == randTable[i]);
>>>>>>> cf7ff489bdf05b06994f3847a169933c74974a03
  }
  delete [] threads;
  cout << "rehashTest: OK" << endl;
  return reHashTable.hashSum.tv_nsec;

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
    assert(spamTable.read(i*4096) == i*4096);
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
    for(auto p : thread_data[i].my_map){
      try{
        if(table.read(p.first) != p.second){
          count = 0;
          for(int j = 0; j < no_write_threads; j++){
            count = count + thread_data[j].my_map.count(p.first);
          }
          if(count > 1){
            //DUPLICATE KEYS
            goto OUT_OF_LOOP;
          }
        }
      } catch(InvalidReadExeption& e){
        cout << "failed to read key " << p.first << endl;
        assert(false);
      }
    assert(table.read(p.first) == p.second);
    OUT_OF_LOOP:;
    }
  }
  delete [] threads;
  delete [] thread_data;
  cout << "megaSpamTest: OK" << endl;
}

void writeReadDeleteTest(){
  cout << "writeReadDeleteTest: RUNNING..." << endl;
  int no_threads = 9;
  int no_write_threads = no_threads/3;
  int no_read_threads = no_threads/3;
  int no_delete_threads = no_threads/3;

  int read_offset = no_write_threads;
  int delete_offset = no_write_threads + no_write_threads;
  int load = 400;
  int i = 0;
  pthread_t* threads = new pthread_t[no_threads];

  mega_data* thread_data = new mega_data[no_threads];
  HashTable<int,int> table = HashTable<int,int>(8);

  for(int i = 0; i < no_threads ; i++){
    thread_data[i].table = &table;
  }
  int value;
  int key;
  // gen map to be written,read and deleted
  for(i = 0; i < no_write_threads; i++ ) {
    for(int j = 0; j < load; j++){
      value = rand();
      do {
        key = rand();
      }while(thread_data[i].my_map.count(key) > 0);
      thread_data[i].my_map.insert(make_pair(key,value));
    }
    thread_data[i + read_offset].my_map = thread_data[i].my_map;
    thread_data[i + delete_offset].my_map = thread_data[i].my_map;
  }

  for(i = 0; i < no_write_threads; i++ ) {
    pthread_create(&threads[i], NULL, &mapWrite, (void*)&thread_data[i]);
    pthread_create(&threads[i + read_offset], NULL, &mapRead, (void*)&thread_data[i + read_offset]);
    pthread_create(&threads[i + delete_offset], NULL, &mapDelete, (void*)&thread_data[i + delete_offset]);
  }

  for (i = 0; i < no_threads; i++){
      pthread_join(threads[i], NULL);
  }

  int count = 0;
  for(int i = 0; i < no_write_threads; i++){
    for(auto p : thread_data[i].my_map){
      try{
        if(table.read(p.first) != p.second){
          count = 0;
          for(int j = 0; j < no_write_threads; j++){
            count = count + thread_data[j].my_map.count(p.first);
          }
          if(count > 1){
            goto OUT_OF_LOOP; //DUPLICATE KEYS
          }
        }
      }catch(InvalidReadExeption& e){   // key must have been removed by delete thread
          for(int j = 0; j < no_delete_threads; j++){
            if(thread_data[j + delete_offset].deleted.count(p.first) > 0){
              goto OUT_OF_LOOP;
            }
          }
          cout << "key " << p.first << " is not in list and was not removed" << endl;
          assert(false);
      }
      assert(table.read(p.first) == p.second);
      OUT_OF_LOOP:;
    }
  }

  delete [] threads;
  delete [] thread_data;
  cout << "writeReadDeleteTest: OK" << endl;
}


//-----------------------------------MAIN-------------------------------------//

int main(){

  //writeAndReadTest();
  uint64_t sum = 0;
  for(int i = 0; i < 1000; i++){
    sum += reHashTest();
  }
  cout <<( sum / 1000 )<< endl;
  //spamBucketTest();
  //megaSpamTest();
  //writeReadDeleteTest();
  delete [] randTable;

}
