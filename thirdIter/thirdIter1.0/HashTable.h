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
#include "Arena/Arena.h"

using namespace std;

template<class Key, class Value> class HashTableIterator;

#define test (1)

template<class Key, class Value> class HashTable {
  friend class HashTableIterator<Key,Value>;

  public:
    HashTable(size_t size);
    ~HashTable();
    HashTable(const HashTable<Key, Value> &table);
    void singleWrite(Key key, Value value);
    Value singleRead(Key key);
    bool readAndWrite(Key key, Value new_val, Value old_val);
    // should be changed to Value readAndWrite(Key key, Value val); set the new value and return the old one

    size_t size();
    size_t getCapacity();
    size_t hash_func(Key key);
    bool remove(Key key);
    bool contains(const Value value); // TODO: this could be parallised
    vector<Key> getKeys(const Value value); // TODO: this could be parallised
    bool containsKey(const Key key);
    void print();
    bool empty();
    void rehash();
    /*TODO a method called readOnly() could be implemented, this make the hashTable only accessable to  reads.
    writes and deletes are not allowed. thus during reads the table dosen't  have to be locked. writes and deletes could be buffered
    and added to the table when the read only phase ends.
    */

    HashTableIterator<Key,Value> begin();
	  HashTableIterator<Key,Value> end();

    #if test
      struct timespec readStart, readEnd, readSum,
        writeStart, writeEnd,writeSum ,funcStart, funcEnd, funcSum,
        rehashStart, rehashEnd, rehashSum, memoryStart, memoryEnd, memorySum,
        hashStart, hashEnd, hashSum;
    #endif

  private:

    //attributes
    Bucket<Key,Value>* buckets;
    size_t capacity;
    atomic<uint16_t> load;
    const size_t cores = get_nprocs_conf();
    mutable shared_timed_mutex rehash_mutex;
    atomic_flag rehash_flag = ATOMIC_FLAG_INIT;
    //Arena* arena;

    //help functions
    void privateRehash();
    static void* subHash(void *argStruct);

};



#endif
