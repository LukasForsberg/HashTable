#include "HashTable.h"

template<class Key, class Value>
HashTable<Key, Value>::HashTable(){
  buckets.resize(128);
}

template<class Key, class Value>
void HashTable<Key, Value>::singleWrite(Key key, Value value){
  int index = hash_func(key);
  buckets[index].push_front(make_pair(key, value));
}

template<class Key, class Value>
const Value* HashTable<Key, Value>::singleRead(Key key){

  size_t index = hash_func(key);

  if (buckets[index].empty()){
    return nullptr;
  } else{
    auto it = buckets[index].begin();
    auto end = buckets[index].end();
    while(it != end){
      if(key == it->first){
        return &it->second;
      }
      it++;
    }
  }
  return nullptr;
}

template<class Key, class Value>
size_t HashTable<Key, Value>::hash_func(Key key){
  hash<Key> h;
  return (h(key) % buckets.size());
}

//wtf??????
template class HashTable<int,int>;
template class HashTable<string,int>;
template class HashTable<int,string>;
template class HashTable<string,string>;
