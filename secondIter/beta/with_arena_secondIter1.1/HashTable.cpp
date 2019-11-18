#include "HashTable.h"
#include <type_traits>

template<class Key, class Value>
HashTable<Key, Value>::HashTable(size_t size){
  buckets = new Bucket<Key,Value>[size];
  capacity = size;
  load = 0;
  rehash_flag = false;
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
  {
    shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
    int index = hash_func(key);
    auto bucket = &buckets[index];
    auto node = bucket->getNode();
    {
      std::unique_lock<std::shared_timed_mutex> lock(*(bucket->getMutex()));
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
        load.fetch_add(1, std::memory_order_relaxed);
      }
    }
  }
  // this is actually 3 times faster!!! equal to load/capacity > 0.625
  if(load.load(memory_order_relaxed) > ( (capacity >> 1) + (capacity >> 2) - (capacity >> 3) ) ){
    // exsclusive lock,
    size_t old_capacity = capacity;
    //check so previus capacity haven't increased.
    rehash_mutex.lock();
    if(old_capacity == capacity){
      rehash();
    }
    rehash_mutex.unlock();
  }
}

template<class Key, class Value>
Value HashTable<Key, Value>::singleRead(Key key){
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
  size_t index = hash_func(key);
  auto bucket = &buckets[index];
  auto node = bucket->getNode();
  {
    std::shared_lock<std::shared_timed_mutex> lock(*(bucket->getMutex()));
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
}

template<class Key, class Value>
void HashTable<Key, Value>::remove(Key key){
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
  auto index = hash_func(key);
  Bucket<Key,Value>* bucket = &buckets[index];
  HashNode<Key,Value>* node = bucket->getNode();
  HashNode<Key,Value>* prev_node = nullptr;
  {
    std::unique_lock<std::shared_timed_mutex> lock(*(bucket->getMutex()));
    while(node != nullptr){
      if(node->getKey() == key){
        if(prev_node != nullptr){
          prev_node->insertNext(node->getNext());
        } else {
          bucket->setNode(node->getNext());
        }
        load.fetch_sub(1, std::memory_order_relaxed);
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
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
  Bucket<Key,Value>* bucket = &buckets[hash_func(key)];
  auto node = bucket->getNode();
  std::shared_lock<std::shared_timed_mutex> lock(*(bucket->getMutex()));
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

template<class Key, class Value>
void HashTable<Key, Value>::checkFlag(shared_lock<shared_timed_mutex>& lock){
  while(rehash_flag){
    cv.wait(lock);
  }
}


//wtf??????
template class HashTable<int,int>;
template class HashTable<string,int>;
template class HashTable<int,string>;
template class HashTable<string,string>;
