#include "../src/Table/HashTable.h"
#include <iostream>
#include <cassert>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <vector>
#include <utility>
#include <map>
#include <atomic>

using namespace std;

static atomic<bool> not_ready = true;

typedef struct w_data{
  HashTable<int,int>* table;
  vector<pair<int,int>> w_vec; // data to be read from table
} w_data;

typedef struct r_data{
  HashTable<int,int>* table;
  vector<int> r_vec; // data to be read from table
} r_data;

typedef struct d_data{
  HashTable<int,int>* table;
  vector<int> d_vec; // data to be deleted from table
} d_data;

typedef struct all_data{
  HashTable<int,int>* table;
  vector<pair<int,int>> w_vec;
  vector<int> r_vec;
  vector<int> d_vec; // data to be deleted from table
} all_data;

void mapDelete(void *arg){
  d_data* data = (d_data*)arg;
  for(auto p : data->d_vec){
    data->table->remove(p);
  }
}

void mapWrite(void *arg){
  w_data* data = (w_data*)arg;
  for(auto p : data->w_vec){
    data->table->write(p.first, p.second);
  }
}

void mapRead(void *arg){
  r_data* data = (r_data*)arg;
  for(auto p : data->r_vec){
    try{
      data->table->read(p);
    } catch(InvalidReadExeption& e){}
  }
}

void* all_in_one(void *arg){
  all_data* data = (all_data*)arg;
  for(auto p : data->w_vec){
    data->table->write(p.first, p.second);
  }
  for(auto p : data->d_vec){
    data->table->remove(p);
  }
  for(auto p : data->r_vec){
    try{
      data->table->read(p);
    } catch(InvalidReadExeption& e){}
  }
  return arg;
}

/*
void* start_all_in_one(void *arg){
  while(not_ready);
  all_in_one(arg);
  return arg;
}
*/
void* startWrite(void *arg){
  while(not_ready);
  mapWrite(arg);
  return arg;
}

void* startRead(void *arg){
  while(not_ready);
  mapRead(arg);
  return arg;
}

void* startDelete(void *arg){
  while(not_ready);
  mapDelete(arg);
  return arg;
}

uint64_t singlePurposeThreadTest(){
  //cout << "Benchmark test for TM: RUNNING..." << endl;
  int no_threads =10;
  int no_write_threads = 1;
  int no_read_threads = 0;
  int no_delete_threads = 0;

  int thread_load = 10000; // workload for thread
  int start_up_load = 5000; // inital load in hashtable
  int start_size = 128;
  int i;
  pthread_t* w_threads = new pthread_t[no_write_threads];
  pthread_t* r_threads = new pthread_t[no_read_threads];
  pthread_t* d_threads = new pthread_t[no_delete_threads];

  // data to be passed to threads
  w_data* w_thread_data = new w_data[no_write_threads];
  r_data* r_thread_data = new r_data[no_read_threads];
  d_data* d_thread_data = new d_data[no_delete_threads];

  /*
  cout << "Starting Up!" << endl;
  cout << "Write threads: " << no_write_threads << endl;
  cout << "Read threads: " << no_read_threads << endl;
  cout << "Delete threads: " << no_delete_threads << endl;
  cout << "thread load: " << thread_load << endl;
  cout << "Table start up load: " << start_up_load << endl;
  cout << "Table start up size: " << start_size << endl;
  cout << endl;
  */


  //shared hashtable
  HashTable<int,int> table = HashTable<int,int>(start_size);

  // set shared table in data passed to threads
  for(i = 0; i < no_write_threads; i++){
    w_thread_data[i].table = &table;
  }
  for(i = 0; i < no_read_threads; i++){
    r_thread_data[i].table = &table;
  }
  for(i = 0; i < no_delete_threads; i++){
    d_thread_data[i].table = &table;
  }

  vector<pair<int,int>> gen_data;
  int value;
  int key;

  // gen map to be written by write treads.
  for(i = 0; i < no_write_threads; i++ ) {
    for(int j = 0; j < thread_load; j++){
      value = rand();
      key = rand();
      w_thread_data[i].w_vec.push_back(make_pair(key,value));
      gen_data.push_back(make_pair(key,value));
    }
  }

  // gen some Key-Values to be in HashTable at start
  for(i = 0; i < start_up_load; i++ ) {
    value = rand();
    key = rand();
    gen_data.push_back((make_pair(key,value)));
    table.write(key,value);
  }
  i = 0;
  // set data to be read by read-threads
  int count = 0;
  while(count < thread_load * no_read_threads){
    for(auto pair : gen_data ){
      r_thread_data[i].r_vec.push_back(pair.first);
      i++;
      count++;
      if(i == no_read_threads){
        i = 0;
      }
      if(count > thread_load * no_read_threads){
        break;
      }
    }
  }

  // set some random data to be deleted by delete threads
  for(i = 0; i < no_delete_threads; i++){
    for(int j = 0; j < thread_load; j++){
      d_thread_data[i].d_vec.push_back(gen_data[rand() % gen_data.size()].first);
    }
  }

  for(i = 0; i < no_write_threads; i++ ) {
    pthread_create(&w_threads[i], NULL, &startWrite, (void*)&w_thread_data[i]);
  }
  for(i = 0; i < no_read_threads; i++ ) {
    pthread_create(&r_threads[i], NULL, &startRead, (void*)&r_thread_data[i]);
  }
  for(i = 0; i < no_delete_threads; i++ ) {
    pthread_create(&d_threads[i], NULL, &startDelete, (void*)&d_thread_data[i]);
  }


  struct timespec start, end,sum;
  // cout << "ready to go!" << endl;

  clock_gettime(CLOCK_REALTIME, &start);
  not_ready = false;

  for(i = 0; i < no_write_threads; i++ ) {
    pthread_join (w_threads[i], NULL);
  }
  for(i = 0; i < no_read_threads; i++ ) {
    pthread_join (r_threads[i], NULL);
  }
  for(i = 0; i < no_delete_threads; i++ ) {
    pthread_join (d_threads[i], NULL);
  }

  clock_gettime(CLOCK_REALTIME, &end);

  sum.tv_nsec = end.tv_nsec - start.tv_nsec;
  sum.tv_sec = end.tv_sec - start.tv_sec;
  uint64_t ret_sum = sum.tv_sec*100000 + sum.tv_nsec/1000;

  // cout << "Time: " << ret_sum <<  " microseconds" << endl;

  delete [] r_threads;
  delete [] w_threads;
  delete [] d_threads;

  delete [] w_thread_data;
  delete [] r_thread_data;
  delete [] d_thread_data;

  return ret_sum;
}


int64_t multiPurposeThreadTest(int no_threads){
  //cout << "Benchmark test for TM: RUNNING..." << endl;

  int thread_load = 100000/no_threads; // workload for thread, read is multiplied by 8
  int start_up_load = 200000; // inital load in hashtable
  int start_size = 524288;
  int i;
  pthread_t* threads = new pthread_t[no_threads];

  // data to be passed to threads
  all_data* thread_data = new all_data[no_threads];
/*
  cout << "Starting Up!" << endl;
  cout << "threads: " << no_threads << endl;
  cout << "thread load: " << thread_load << endl;
  cout << "Table start up load: " << start_up_load << endl;
  cout << "Table start up size: " << start_size << endl;
  cout << endl;
*/

  //shared hashtable
  HashTable<int,int> table = HashTable<int,int>(start_size);

  // set shared table in data passed to threads
  for(i = 0; i < no_threads; i++){
    thread_data[i].table = &table;
  }

  vector<pair<int,int>> gen_data;
  int value;
  int key;

  //gen some data at start
  gen_data.reserve(no_threads * thread_load + start_up_load);
  for(i = 0; i < start_up_load; i++ ) {
    value = rand();
    gen_data.push_back((make_pair(value,value)));
    table.write(value,value);
  }

  // set some data to be deleted by delete threads
  int count = 0;
  for(i = 0; i < no_threads; i++){
    thread_data[i].d_vec.reserve(thread_load);
    for (auto it = gen_data.begin(); it != gen_data.end();  ++it ){
      thread_data[i].d_vec.push_back((*it).first);
      count++;
      if(count == thread_load){
        gen_data.erase(gen_data.begin(), it);
        count = 0;
        break;
      }
    }
  }

  for(i = 0; i < no_threads; i++){
    thread_data[i].r_vec.reserve(thread_load * 8);
  }
  // set data to be read by read-threads
  i = 0;
  count = 0;
  int max =  thread_load * no_threads * 8;
  while(count < max){
    for(auto pair : gen_data ){
      thread_data[i].r_vec.push_back(pair.first);
      i++;
      count++;
      if(i == no_threads){
        i = 0;
      }
      if(count > max){
        break;
      }
    }
  }

  // gen map to be written by write treads.
  for(i = 0; i < no_threads; i++ ) {
    thread_data[i].w_vec.reserve(thread_load);
    for(int j = 0; j < thread_load; j++){
      key = rand();
      thread_data[i].w_vec.push_back(make_pair(key,key));
      gen_data.push_back(make_pair(key,key));
    }
  }

  for(i = 0; i < no_threads; i++ ) {
    pthread_create(&threads[i], NULL, &all_in_one, (void*)&thread_data[i]);
  }

  struct timespec start, end,sum;
  // cout << "ready to go!" << endl;

  clock_gettime(CLOCK_REALTIME, &start);
  //not_ready = false;

  for(i = 0; i < no_threads; i++ ) {
    pthread_join (threads[i], NULL);
  }
  //not_ready = true;

  clock_gettime(CLOCK_REALTIME, &end);

  sum.tv_nsec = end.tv_nsec - start.tv_nsec;
  sum.tv_sec = end.tv_sec - start.tv_sec;
  int64_t ret_sum = sum.tv_sec*100000 + sum.tv_nsec/1000;

  // cout << "Time: " << ret_sum <<  " microseconds" << endl;

  delete [] threads;
  delete [] thread_data;
  return ret_sum;
}



int main(){
  //int no_threads = 1;
  cout << "Benchmark test for " << " Single Thread: RUNNING..." << endl;
  vector<int> v = {1};
  int64_t sum = 0;
  int64_t temp;
  int64_t runs = 200;

  for(int no_threads : v){
    sum = 0;
    cout << no_threads  << " threads "<< endl;
    for(int i = 0; i < runs; i++){
      //temp = singlePurposeThreadTest();
      temp = multiPurposeThreadTest(no_threads);
      if(temp > 84467440737090352 || temp < 0){
        i--;
      } else {
        sum = sum + temp;
      }
      //cout << i << endl;
    }
    cout << "AVG :" << sum/runs << endl;
    cout << endl;
  }

  return 0;
}
