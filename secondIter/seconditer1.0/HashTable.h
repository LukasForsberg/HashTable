#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <cstdlib>
#include "InvalidReadException.h"
#include "HashNode.h"

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

    #if test
      struct timespec start, end,totStart,totEnd, funcTime, totTime;
    #endif
  private:
    HashNode<Key,Value>** buckets;
    size_t capacity;
    size_t load;
    void rehash();



};



#endif
