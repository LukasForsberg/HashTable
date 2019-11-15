#include "HashTable.h"
#include <type_traits>

template<class Key, class Value>
HashTable<Key, Value>::HashTable(size_t size){
  buckets = new Bucket<Key,Value>[size];
  capacity = size;
  load = 0;
  hash_flag = false;
  active_users = 0;
}

template<class Key, class Value>
HashTable<Key, Value>::~HashTable(){
  HashNode<Key,Value>* node;
  HashNode<Key,Value>* temp;
  for(size_t i = 0; i < capacity; i++){
    node = buckets[i].getNode();
    while( node != nullptr){
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
  auto bucket = &buckets[index];
  auto node = bucket->getNode();
  {
    std::unique_lock<std::shared_timed_mutex> lock(*(bucket->getMutex()));
    checkFlag(lock);
    active_users.fetch_add(1, std::memory_order_relaxed);
    while(node != nullptr){
      if(key == node->getKey()){
         node->setValue(value);
         break;
      }
      node = node->getNext();
    }
    if(node == nullptr){
      node = new HashNode<Key,Value>(key,value);
      bucket->append(node);
      this->load++;
    }
  }
  active_users.fetch_sub(1, std::memory_order_relaxed);
  // this is actually 3 times faster!!! equal to load/capacity > 0.625
  if(load > ( (capacity >> 1) + (capacity >> 2) - (capacity >> 3) ) ){
    if(!atomic_flag_test_and_set_explicit(&rehash_lock, memory_order_consume)){
      rehash();
    }
  }
}

template<class Key, class Value>
Value HashTable<Key, Value>::singleRead(Key key){

  size_t index = hash_func(key);
  auto bucket = &buckets[index];
  auto node = bucket->getNode();
  {
    std::shared_lock<std::shared_timed_mutex> lock(*(bucket->getMutex()));
    checkFlag(lock);
    active_users.fetch_add(1, std::memory_order_relaxed);
    if (node == nullptr){
      active_users.fetch_sub(1, std::memory_order_relaxed);
      throw InvalidReadExeption();
    } else {
      do{
        if(key == node->getKey()){
          active_users.fetch_sub(1, std::memory_order_relaxed);
          return node->getValue();
        }
        node = node->getNext();
      } while(node != nullptr);
        active_users.fetch_sub(1, std::memory_order_relaxed);
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
  Bucket<Key,Value>* temp = new Bucket<Key,Value>[capacity];
  HashNode<Key,Value>* node;
  for(size_t i = 0; i < old_capacity; i++){
    node = buckets[i].getNode();
    while(node != nullptr){
      temp[hash_func(node->getKey())].append(node);
      node = node->getNext();
    }
  }
  delete [] buckets;
  buckets = temp;
  atomic_flag_clear_explicit(&rehash_lock, std::memory_order_release);
  cv.notify_all();
}

template<class Key, class Value>
void HashTable<Key, Value>::remove(Key key){
  auto index = hash_func(key);
  Bucket<Key,Value>* bucket = &buckets[index];
  HashNode<Key,Value>* node = bucket->getNode();
  HashNode<Key,Value>* prev_node = nullptr;
  {
    std::unique_lock<std::shared_timed_mutex> lock(*(bucket->getMutex()));
    checkFlag(lock);
    active_users.fetch_add(1, std::memory_order_relaxed);
    while(node != nullptr){
      if(node->getKey() == key){
        if(prev_node != nullptr){
          prev_node->insertNext(node->getNext());
        } else {
          bucket->setNode(node->getNext());
        }
        load--;
        delete node;
        active_users.fetch_sub(1, std::memory_order_relaxed);
        return;
      }
      prev_node = node;
      node = node->getNext();
    }
  }
}

template<class Key, class Value>
bool HashTable<Key, Value>::contains(const Key key){
  Bucket<Key,Value>* bucket = &buckets[hash_func(key)];
  auto node = bucket->getNode();
  std::unique_lock<std::shared_timed_mutex> lock(*(bucket->getMutex()));
  checkFlag(lock);
  active_users.fetch_add(1, std::memory_order_relaxed);
  while (node != nullptr){
    if(node->getKey() == key){
      active_users.fetch_sub(1, std::memory_order_relaxed);
      return true;
    }
  }
  active_users.fetch_sub(1, std::memory_order_relaxed);
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

template<class Key, class Value>
void HashTable<Key, Value>::checkFlag(shared_lock<std::shared_timed_mutex>& lock){
  while(rehash_lock){
    cv.wait(lock);
  }
}

template<class Key, class Value>
void HashTable<Key, Value>::checkFlag(unique_lock<std::shared_timed_mutex>& lock){
  while(rehash_lock){
    cv.wait(lock);
  }
}

//wtf??????
template class HashTable<int,int>;
template class HashTable<string,int>;
template class HashTable<int,string>;
template class HashTable<string,string>;
