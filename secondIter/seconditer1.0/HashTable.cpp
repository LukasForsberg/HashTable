#include "HashTable.h"
#include <type_traits>

template<class Key, class Value>
HashTable<Key, Value>::HashTable(size_t size){
  buckets = new Bucket<Key,Value>[size];
  for(size_t i = 0; i < size; i++){
    buckets[i] = Bucket();
  }
  capacity = size;
  load = 0;
}

template<class Key, class Value>
HashTable<Key, Value>::~HashTable(){
  Bucket* node;
  Bucket* temp;
  for(size_t i = 0; i < capacity; i++){
    node = &buckets[i];
    while(node != nullptr){
      temp = node->getNext();
      delete node;
      node = temp;
    }
  }
  delete [] buckets;
}

template<class Key, class Value>
void HashTable<Key, Value>::singleWrite(Key key, Value value){
  int index = hash_func(key);
  auto node = buckets[index];
  {
    std::unique_lock lock(*(node->getMutex()));
    while(node != nullptr){
      if(key == node->getKey()){
         node->setValue(value);
         break;
      }
      node = node->getNext();
    }
    if(node == nullptr){
      node = new HashNode<Key,Value>(key,value);
      node->insertNext(buckets[index]);
      buckets[index] = node;
      this->load++;
    }
  }
  // this is actually 3 times faster!!! equal to load/capacity > 0.625
  if(load > ( (capacity >> 1) + (capacity >> 2) - (capacity >> 3) ) ){
    rehash();
  }
}

template<class Key, class Value>
Value HashTable<Key, Value>::singleRead(Key key){

  size_t index = hash_func(key);
  auto node = buckets[index];
  {
    std::shared_lock lock(*(node->getMutex()));
    if (node == nullptr){
      throw InvalidReadExeption();
    } else {
      do{
        if(key == node->getKey()){
          return node->getValue();
        }
        node = node->getNext();
      } while(node != nullptr);
        throw InvalidReadExeption();
      }
  }
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
  size_t old_capacity = capacity;
  capacity = capacity << 1;
  HashNode<Key,Value>** temp = new HashNode<Key,Value>*[capacity];
  for(size_t i = 0; i < capacity; i++){
    temp[i] = nullptr;
  }
  HashNode<Key,Value>* node;
  for(size_t i = 0; i < old_capacity; i++){
    node = buckets[i];
    while(node != nullptr){
      temp[hash_func(node->getKey())] = node;
      node = node->getNext();
    }
  }
  delete [] buckets;
  buckets = temp;
}

template<class Key, class Value>
void HashTable<Key, Value>::remove(Key key){
  auto index = hash_func(key);
  HashNode<Key,Value>* node = buckets[index];
  HashNode<Key,Value>* prev_node = nullptr;
  {
    std::unique_lock lock(*(node->getMutex()));
    while(node != nullptr){
      if(node->getKey() == key){
        if(prev_node != nullptr){
          prev_node->insertNext(node->getNext());
        } else {
          buckets[index] = node->getNext();
        }
        load--;
        delete node;
        return;
      }
      prev_node = node;
      node = node->getNext();
    }
  }
}

template<class Key, class Value>
bool HashTable<Key, Value>::contains(const Key key){
  HashNode<Key,Value>* node = buckets[hash_func(key)];
  while (node != nullptr){
    if(node->getKey() == key){
      return true;
    }
  }
  return false;
}


template<class Key, class Value>
size_t HashTable<Key, Value>::size(){
  return load;
}

template<class Key, class Value>
size_t HashTable<Key, Value>::getCapacity(){
  return capacity;
}


//wtf??????
template class HashTable<int,int>;
template class HashTable<string,int>;
template class HashTable<int,string>;
template class HashTable<string,string>;
