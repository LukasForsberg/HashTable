#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <cstdlib>
#include "InvalidReadException.h"
#include "HashNode.h"
#include "Bucket.h"
#include <thread>
#include <atomic>
#include "InvalidSizeException.h"
#include <sys/sysinfo.h>
#include "HashTableIterator.h"
#include <vector>

using namespace std;

template<class Value> class HashTableIterator;

#define test (1)

template<class Value> class HashTable {
  friend class HashTableIterator<Value>;

  public:
    HashTable(size_t size);
    ~HashTable();
    HashTable(const HashTable<Value> &table);
    void singleWrite(uint64_t key, Value value);
    Value singleRead(uint64_t key);
    bool readAndWrite(uint64_t key, Value new_val, Value old_val);

    size_t size();
    size_t getCapacity();
    size_t hash_func(uint64_t key);
    bool remove(uint64_t key);
    bool contains(const Value value); // TODO: this could be parallised
    vector<uint64_t> getKeys(const Value value); // TODO: this could be parallised
    bool containsKey(const uint64_t key);
    void print();
    bool empty();
    void rehash();

    HashTableIterator<Value> begin();
	  HashTableIterator<Value> end();

    #if test
      struct timespec readStart, readEnd, readSum,
        writeStart, writeEnd,writeSum ,funcStart, funcEnd, funcSum,
        rehashStart, rehashEnd, rehashSum, memoryStart, memoryEnd, memorySum;
    #endif

  private:

    //attributes
    Bucket<Value>* buckets;
    uint64_t capacity;
    atomic<uint64_t> load;
    const uint16_t cores = get_nprocs_conf();
    mutable shared_timed_mutex rehash_mutex;
    atomic_flag rehash_flag = ATOMIC_FLAG_INIT;

    //help functions
    void privateRehash();
    static void* subHash(void *argStruct);

};



#endif
