#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <vector>
#include <utility>
#include <cstdlib>
#include <functional>
#include <forward_list>

using namespace std;

template<class Key, class Value> class HashTable {

  public:
    HashTable();
    void singleWrite(Key& key, Value& value);
    const Value* singleRead(Key& key);
    size_t size();
    size_t hash_func(Key& key);

  private:
    vector<forward_list<pair<Key,Value>>> buckets;

};



#endif
