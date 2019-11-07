#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <vector>
#include <utility>
#include <cstdlib>
#include <functional>
#include <forward_list>

#define test (0)

using namespace std;

template<class Key, class Value> class HashTable {

  public:
    HashTable(size_t size);

    void singleWrite(Key key, Value value);

    const Value* singleRead(Key key);
    size_t size();
    size_t hash_func(Key key);
    void remove(Key key);
    #if test
      struct timespec start, end,totStart,totEnd, funcTime, totTime;
    #endif
  private:
    vector<forward_list<pair<Key,Value>>> buckets;
    size_t capacity;
    size_t load;
    void rehash();



};



#endif
