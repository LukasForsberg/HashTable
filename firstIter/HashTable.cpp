#include "HashTable.h"

template<class Key, class Value>
HashTable<Key, Value>::HashTable(size_t size){
  buckets.resize(size);
  capacity = size;
  load = 0;
}

template<class Key, class Value>
void HashTable<Key, Value>::singleWrite(Key key, Value value){
  int index = hash_func(key);

  buckets[index].push_front(make_pair(key, value));
  load++;
  // this is actually 3 times faster!!! equal to load/capacity > 0.625
  if(load > ( (capacity >> 1) + (capacity >> 2) - (capacity >> 3) ) ){
    rehash();
  }
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
  #if test
    clock_gettime(CLOCK_REALTIME, &totEnd);
    totTime.tv_nsec = totTime.tv_nsec + totEnd.tv_nsec - totStart.tv_nsec;
    clock_gettime(CLOCK_REALTIME, &start);
  #endif
  hash<Key> h;
  auto r = (h(key) % capacity);
  #if test
    clock_gettime(CLOCK_REALTIME, &end);
    funcTime.tv_nsec = funcTime.tv_nsec + end.tv_nsec - start.tv_nsec;
    clock_gettime(CLOCK_REALTIME, &totStart);
  #endif
  return r;
}

template<class Key, class Value>
void HashTable<Key, Value>::rehash(){

  auto temp = vector<forward_list<pair<Key,Value>>>();
  capacity = capacity << 1;
  temp.resize(capacity);

  for(auto bucket : buckets){
    auto it = bucket.begin();
    auto end = bucket.end();
    while(it != end){
      temp[hash_func(it->first)].push_front(make_pair(it->first, it->second));
      it++;
    }
  }
  buckets = temp;
}

template<class Key, class Value>
void HashTable<Key, Value>::remove(Key key){
  buckets[hash_func(key)].remove_if([&](const pair<Key, Value>& obj)
                              { return obj.first == key; });

}


template<class Key, class Value>
size_t HashTable<Key, Value>::size(){
  return capacity;
}


//wtf??????
template class HashTable<int,int>;
template class HashTable<string,int>;
template class HashTable<int,string>;
template class HashTable<string,string>;
