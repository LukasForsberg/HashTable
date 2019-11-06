#include "HashTable.h"

template<class Key, class Value>
HashTable::HashTable(){
  buckets = new vector<forward_list<Pair<Key,Value>>(128);
}

template<class Key, class Value>
void HashTable::singleWrite(Key key, Value value){

  int index = hash_func(key);

  buckets[index].push_front(Pair<Key, Value>(key, value));

}

template<class Key, class Value>
Value* HashTable::singleRead(Key key){

  size_t index = hash_func(key);

  if (buckets[index].isEmpty()){
    return nullptr;
  } else{
    for(auto it : buckets[index]){

      if(key == it.first){
        return &it.second;
      }
    }
    return nullptr;
  }
}

size_t HashTable::hash_func(Key key){
  hash<Key> h;
  return h(key);
}
