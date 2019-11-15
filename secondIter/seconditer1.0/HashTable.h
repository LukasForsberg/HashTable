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

    void checkFlag(shared_lock<std::shared_timed_mutex>& lock);
    void checkFlag(unique_lock<std::shared_timed_mutex>& lock);

    #if test
      struct timespec start, end,totStart,totEnd, funcTime, totTime;
    #endif
  private:
    Bucket<Key,Value>* buckets;
    size_t capacity;
    size_t load;
    void rehash();

    atomic_flag rehash_lock = ATOMIC_FLAG_INIT;
    atomic<uint16_t> active_users;
    condition_variable_any cv;



};



#endif
