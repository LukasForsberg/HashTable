#include <iostream>
#include <vector>
#include <utility>
#include <cstdlib>
#include <functional>
#include <forward_list>

using namespace std;

class HashTable {

  public:
    template<class Key, class Value>
    void singleWrite(Key key, Value value);

    template<class Key, class Value>
    Value* singleRead(Key key);

    size_t size();
    HashTable();

    template<class Key, class Value>
    size_t hash_func(Key key);

  private:
    template<class Key, class Value>
    vector<forward_list<Pair<Key,Value>>> buckets;
};
