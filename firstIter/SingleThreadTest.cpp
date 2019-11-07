#include "HashTable.h"
#include <iostream>
#include <string>
#include <cassert>
#include <stdlib.h>
#include <time.h>

void test1(){
  //----------------------------------------------------------------------------
  //Test 1: Simple write->read.

  HashTable<string,int> strTable = HashTable<string,int>(128);

  strTable.singleWrite("Edvin", 2);

  assert(*strTable.singleRead("Edvin") == 2);

  cout << "Test 1: OK" << endl;
}

void test2(){
  //----------------------------------------------------------------------------
  //Test2: Writing to same bucket.

  HashTable<int,int> intTable = HashTable<int,int>(128);

  int first_int = intTable.size();
  int second_int = intTable.size() * 2;

  intTable.singleWrite(first_int, 13);
  intTable.singleWrite(second_int, 14);

  assert(*intTable.singleRead(first_int) == 13);
  assert(*intTable.singleRead(second_int) == 14);

  cout << "Test 2: OK" << endl;
}

void test3(){
  //----------------------------------------------------------------------------
  //Test3: Causing a rehash.

  HashTable<int,int> hashTable = HashTable<int,int>(128);
  srand (time(0));

  for(int i = 0; i < 100; i++){
    hashTable.singleWrite(rand() % 100 + 1, rand() % 100 + 1);
  }
  assert(hashTable.size() == 256);
  cout << "Test 3: OK"  << endl;
}

void test4(){
  HashTable<int,int> hashTable = HashTable<int,int>(128);
  srand (time(0));

  for(int i = 0; i < 50; i++){
    hashTable.singleWrite(i, rand() % 100 + 1);
  }
  assert(hashTable.size() == 128);
  assert(hashTable.singleRead(5) != nullptr);
  hashTable.remove(5);
  assert(hashTable.singleRead(5) == nullptr);
  cout << "Test 4: OK" << endl;
}

void test5(){
  HashTable<int,int> hashTable = HashTable<int,int>(256);
  srand (time(0));
  struct timespec start, end;

  clock_gettime(CLOCK_REALTIME, &start);
  for(int i = 0; i < 100; i++){
    hashTable.singleWrite(i, i);
  }
  for(int i = 0; i < hashTable.size(); i++){
    hashTable.singleRead(i);
  }
  clock_gettime(CLOCK_REALTIME, &end);
  //double time_spent = (end.tv_sec - start.tv_sec) +
  cout << "Test 5: init size 256 took: " << (end.tv_nsec - start.tv_nsec) << endl;
}

void test6(){
  HashTable<int,int> hashTable = HashTable<int,int>(128);
  srand (time(0));
  struct timespec start, end;

  clock_gettime(CLOCK_REALTIME, &start);
  for(int i = 0; i < 100; i++){
    hashTable.singleWrite(i, i);
  }
  for(int i = 0; i < hashTable.size(); i++){
    hashTable.singleRead(i);
  }
  clock_gettime(CLOCK_REALTIME, &end);
  //double time_spent = (end.tv_sec - start.tv_sec) +
  cout << "Test 6: init size 128 took: " << (end.tv_nsec - start.tv_nsec) << endl;
}

void test7(){
  HashTable<int,int> hashTable = HashTable<int,int>(64);
  srand (time(0));
  struct timespec start, end;

  clock_gettime(CLOCK_REALTIME, &start);
  for(int i = 0; i < 100; i++){
    hashTable.singleWrite(i, i);
  }
  for(int i = 0; i < hashTable.size(); i++){
    hashTable.singleRead(i);
  }
  clock_gettime(CLOCK_REALTIME, &end);
  //double time_spent = (end.tv_sec - start.tv_sec) +
  cout << "Test 7: init size 64 took: " << (end.tv_nsec - start.tv_nsec) << endl;
}

void test8(){
  HashTable<int,int> hashTable = HashTable<int,int>(256);
  srand (time(0));
  struct timespec startWrite, endWrite, startRead, endRead;

  clock_gettime(CLOCK_REALTIME, &startWrite);
  for(int i = 0; i < 100; i++){
    hashTable.singleWrite(i, i);
  }
  clock_gettime(CLOCK_REALTIME, &endWrite);
  clock_gettime(CLOCK_REALTIME, &startRead);
  for(int i = 0; i < hashTable.size(); i++){
    hashTable.singleRead(i);
  }
  clock_gettime(CLOCK_REALTIME, &endRead);
  //double time_spent = (end.tv_sec - start.tv_sec) +
  cout << "Test 8: write took : " << (endWrite.tv_nsec - startWrite.tv_nsec) << "  Read took: " << (endRead.tv_nsec - startRead.tv_nsec) << endl;
}

void test9(){
  #if test
  HashTable<int,int> hashTable = HashTable<int,int>(256);
  srand (time(0));
  hashTable.funcTime.tv_nsec = 0;
  hashTable.totTime.tv_nsec = 0;
  clock_gettime(CLOCK_REALTIME, &hashTable.totStart);
  for(int i = 0; i < 100; i++){
    hashTable.singleWrite(i, i);
  }
  for(int i = 0; i < hashTable.size(); i++){
    hashTable.singleRead(i);
  }
  clock_gettime(CLOCK_REALTIME, &hashTable.totEnd);
  hashTable.totTime.tv_nsec = hashTable.totTime.tv_nsec + hashTable.totEnd.tv_nsec - hashTable.totStart.tv_nsec;
  //double time_spent = (end.tv_sec - start.tv_sec) +
  cout << "Test 9: total execution took : " << (hashTable.totTime.tv_nsec) << "  Hash func took: " << (hashTable.funcTime.tv_nsec) << endl;
  double t = (long double)hashTable.funcTime.tv_nsec/(hashTable.funcTime.tv_nsec + hashTable.totTime.tv_nsec);
  cout << "        hash_funs was " << t << " of total execution" << endl;
  #endif
}

int main(){

  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
  test7();
  test8();
  #if test
    test9();
  #endif
}
