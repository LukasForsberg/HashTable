#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <cstdlib>
#include "InvalidReadException.h"
#include "HashNode.h"
#include "Bucket.h"
#include <atomic>

#define test (1)

using namespace std;

template<class Key, class Value> class HashTable {

  public:
    HashTable(size_t size);
    ~HashTable();

    void singleWrite(Key key, Value value);
    Value singleRead(Key key);

    size_t size();
    size_t getCapacity();
    size_t hash_func(Key key);
    void remove(Key key);
    bool contains(const Key key);
    void print();


    #if test
      struct timespec readStart, readEnd, readSum,
        writeStart, writeEnd,writeSum ,funcStart, funcEnd, funcSum,
        rehashStart, rehashEnd, rehashSum, memoryStart, memoryEnd, memorySum;
    #endif

  private:
    Bucket<Key,Value>* buckets;
    size_t capacity;
    atomic<uint16_t> load;
    void rehash();
    void *subHash(void *argStruct);

    bool rehash_flag;
    mutable shared_timed_mutex rehash_mutex;



};



#endif
