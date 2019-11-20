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

  assert(strTable.singleRead("Edvin") == 2);
  assert(strTable.size() == 1);

  cout << "Test 1: OK" << endl;
}

void test2(){
  //----------------------------------------------------------------------------
  //Test2: Writing to same bucket.

  HashTable<int,int> intTable = HashTable<int,int>(128);

  int first_int = intTable.getCapacity();
  int second_int = intTable.getCapacity() * 2;

  intTable.singleWrite(first_int, 13);
  intTable.singleWrite(second_int, 14);

  assert(intTable.singleRead(first_int) == 13);
  assert(intTable.singleRead(second_int) == 14);
  assert(intTable.size() == 2);

  cout << "Test 2: OK" << endl;
}

void test3(){
  //----------------------------------------------------------------------------
  //Test3: Causing a rehash.

  HashTable<int,int> hashTable = HashTable<int,int>(128);
  srand (time(0));

  for(int i = 0; i < 100; i++){
    hashTable.singleWrite(i, rand() % 100 + 1);
  }
  assert(hashTable.getCapacity() == 256);
  assert(hashTable.size() == 100);
  cout << "Test 3: OK"  << endl;
}

void test4(){
  //----------------------------------------------------------------------------
  //Test4: access removd value
  HashTable<int,int> hashTable = HashTable<int,int>(128);

  for(int i = 0; i < 50; i++){
    hashTable.singleWrite(i, i);
  }
  assert(hashTable.getCapacity() == 128);
  assert(hashTable.singleRead(5) == 5);
  assert(hashTable.size() == 50);
  hashTable.remove(5);
  try {
    hashTable.singleRead(5);
    assert(false);
  }catch(InvalidReadExeption& e){
    assert(true);
  }
  cout << "Test 4: OK" << endl;
}

void test5(){
  //----------------------------------------------------------------------------
  //Test5: check for no duplicate keys
  HashTable<int,int> hashTable = HashTable<int,int>(128);

  for(int i = 0; i < 50; i++){
    hashTable.singleWrite(5, i);
  }
  assert(hashTable.singleRead(5) == 49);
  assert(hashTable.size() == 1);
  cout << "Test 5: OK" << endl;
}

void test6(){
  //----------------------------------------------------------------------------
  //Test6: hashed to same bucket but have different keys
  HashTable<int,int> hashTable = HashTable<int,int>(8);
  srand (time(0));
  hashTable.singleWrite(5, 1);
  int index1 = hashTable.hash_func(5);

  int index2;
  int b;
  do {
    b = rand();
    index2 = hashTable.hash_func(b);
  }while(index1 != index2 || b == 5);
  hashTable.singleWrite(b, 2);

  assert(hashTable.singleRead(5) == 1);
  assert(hashTable.singleRead(b) == 2);
  assert(hashTable.size() == 2);
  cout << "Test 6: OK" << endl;
}

void containsTest(){

  HashTable<string,int> strTable = HashTable<string,int>(128);

  strTable.singleWrite("Edvin", 2);

  assert(strTable.containsKey("Edvin"));
  assert(strTable.contains(2));

  cout << "ContainsTest: OK" << endl;
}

void isEmptyTest(){

  HashTable<string,int> strTable = HashTable<string,int>(128);

  assert(strTable.empty());

  cout << "isEmptyTest: OK" << endl;
}

void manualRehashTest(){

  HashTable<string,int> strTable = HashTable<string,int>(128);

  strTable.rehash();

  assert(strTable.getCapacity() == 128*2);

  strTable.rehash();

  assert(strTable.getCapacity() == 128*4);

  cout << "manualRehashTest: OK" << endl;
}

void copyTest(){

  HashTable<string,int> strTable = HashTable<string,int>(128);

  strTable.singleWrite("Edvin", 96);
  strTable.singleWrite("Lukas", 85);

  HashTable<string,int> copyTable = strTable;

  strTable.singleWrite("Pelle", 44);

  assume


  cout << "copyTest: OK" << endl;
}

void performance_test1(){
  HashTable<int,int> hashTable = HashTable<int,int>(256);
  srand (time(0));
  struct timespec start, end;

  clock_gettime(CLOCK_REALTIME, &start);
  for(int i = 0; i < 100; i++){
    hashTable.singleWrite(i, i);
  }
  for(size_t i = 0; i < 100; i++){
    hashTable.singleRead(i);
  }
  clock_gettime(CLOCK_REALTIME, &end);
  //double time_spent = (end.tv_sec - start.tv_sec) +
  cout << "Performance Test 1: init size 256 took: " << (end.tv_nsec - start.tv_nsec) << endl;
}

void iteratorTest(){
  HashTable<int,int> hashTable = HashTable<int,int>(128);
  for(int i = 0; i < 100; i++){
    hashTable.singleWrite(i, i);
  }
  int i = 0;
  for(auto node : hashTable){
    assert(node.getKey() == i);
    assert(node.getValue() == i);
    i++;
  }
  cout << "Iterator test: OK" << endl;
}

void performance_test2(){
  HashTable<int,int> hashTable = HashTable<int,int>(128);
  srand (time(0));
  struct timespec start, end;

  clock_gettime(CLOCK_REALTIME, &start);
  for(int i = 0; i < 100; i++){
    hashTable.singleWrite(i, i);
  }
  for(size_t i = 0; i < hashTable.size(); i++){
    hashTable.singleRead(i);
  }
  clock_gettime(CLOCK_REALTIME, &end);
  //double time_spent = (end.tv_sec - start.tv_sec) +
  cout << "Performance Test 2: init size 128 took: " << (end.tv_nsec - start.tv_nsec) << endl;
}

void performance_test3(){
  HashTable<int,int> hashTable = HashTable<int,int>(64);
  srand (time(0));
  struct timespec start, end;

  clock_gettime(CLOCK_REALTIME, &start);
  for(int i = 0; i < 100; i++){
    hashTable.singleWrite(i, i);
  }
  for(size_t i = 0; i < hashTable.size(); i++){
    hashTable.singleRead(i);
  }
  clock_gettime(CLOCK_REALTIME, &end);
  //double time_spent = (end.tv_sec - start.tv_sec) +
  cout << "Performance Test 3: init size 64 took: " << (end.tv_nsec - start.tv_nsec) << endl;
}

void performance_test4(){
  HashTable<int,int> hashTable = HashTable<int,int>(256);
  srand (time(0));
  struct timespec startWrite, endWrite, startRead, endRead;

  clock_gettime(CLOCK_REALTIME, &startWrite);
  for(int i = 0; i < 100; i++){
    hashTable.singleWrite(i, i);
  }
  clock_gettime(CLOCK_REALTIME, &endWrite);
  clock_gettime(CLOCK_REALTIME, &startRead);
  for(size_t i = 0; i < hashTable.size(); i++){
    hashTable.singleRead(i);
  }
  clock_gettime(CLOCK_REALTIME, &endRead);
  //double time_spent = (end.tv_sec - start.tv_sec) +
  cout << "Performance Test 4: write took : " << (endWrite.tv_nsec - startWrite.tv_nsec) << "  Read took: " << (endRead.tv_nsec - startRead.tv_nsec) << endl;
}

void performance_test5(){
  #if test
  HashTable<int,int> hashTable = HashTable<int,int>(128);
  srand (time(0));
  hashTable.funcSum.tv_nsec = 0;
  hashTable.writeSum.tv_nsec = 0;
  hashTable.readSum.tv_nsec = 0;
  hashTable.rehashSum.tv_nsec = 0;
  hashTable.memorySum.tv_nsec = 0;

  for(int i = 0; i < 1000; i++){
    hashTable.singleWrite(i, i);
  }
  for(size_t i = 0; i < hashTable.size(); i++){
    hashTable.singleRead(i);
  }

  cout << "write: " <<  hashTable.writeSum.tv_nsec << endl;
  cout << "read: " <<  hashTable.readSum.tv_nsec << endl;
  cout << "hash func: " <<  hashTable.funcSum.tv_nsec << endl;
  cout << "rehash: " <<  hashTable.rehashSum.tv_nsec << endl;
  cout << "memory: " <<  hashTable.memorySum.tv_nsec << endl;

  #endif
}

int main(){

  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
<<<<<<< HEAD
  iteratorTest();
=======
  containsTest();
  manualRehashTest();
  isEmptyTest();


>>>>>>> bdbe21c465e670dff76129931b61ae11a2d65a51
  cout << endl;

  performance_test1();
  performance_test2();
  performance_test3();
  performance_test4();
  #if test
    performance_test5();
  #endif

}
