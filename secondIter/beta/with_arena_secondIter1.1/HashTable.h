#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <cstdlib>
#include "InvalidReadException.h"
#include "HashNode.h"
#include "Bucket.h"
#include <atomic>

#define test (0)

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

    void checkFlag(shared_lock<shared_timed_mutex>& lock);


    #if test
      struct timespec start, end,totStart,totEnd, funcTime, totTime;
    #endif
  private:
    Bucket<Key,Value>* buckets;
    size_t capacity;
    atomic<uint16_t> load;
    void rehash();

    bool rehash_flag;
    mutable shared_timed_mutex rehash_mutex;
    condition_variable_any cv;



};



#endif
