#include "HashTable.h"
#include <type_traits>

template<class Value>
HashTable<Key, Value>::HashTable(size_t size){
  buckets = new Bucket<Key,Value>[size];
  capacity = size;
  load = 0;
}

template<class Value>
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

template<class Value>
void HashTable<Key, Value>::singleWrite(Key key, Value value){
  #if test
    clock_gettime(CLOCK_REALTIME, &writeStart);
  #endif

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
        #if test
          clock_gettime(CLOCK_REALTIME, &memoryStart);
        #endif
        node = new HashNode<Key,Value>(key,value);
        #if test
          clock_gettime(CLOCK_REALTIME, &memoryEnd);
          memorySum.tv_nsec = memorySum.tv_nsec + memoryEnd.tv_nsec - memoryStart.tv_nsec;
        #endif
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
      privateRehash();
    }
    rehash_mutex.unlock();
  }

  #if test
    clock_gettime(CLOCK_REALTIME, &writeEnd);
    writeSum.tv_nsec = writeSum.tv_nsec + writeEnd.tv_nsec - writeStart.tv_nsec;
  #endif
}

template<class Value>
Value HashTable<Key, Value>::singleRead(Key key){
  #if test
    clock_gettime(CLOCK_REALTIME, &readStart);
  #endif
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
          #if test
            clock_gettime(CLOCK_REALTIME, &readEnd);
            readSum.tv_nsec = readSum.tv_nsec + readEnd.tv_nsec - readStart.tv_nsec;
          #endif
          return node->getValue();
        }
        node = node->getNext();
      } while(node != nullptr);
        throw InvalidReadExeption();
      }
  }
}


template<class Value>
size_t HashTable<Key, Value>::hash_func(Key key){
  #if test
    clock_gettime(CLOCK_REALTIME, &funcStart);
  #endif
  hash<Key> h;
  auto r = (h(key) % capacity);
  #if test
    clock_gettime(CLOCK_REALTIME, &funcEnd);
    funcSum.tv_nsec = funcSum.tv_nsec + funcEnd.tv_nsec -funcStart.tv_nsec;
  #endif
  return r;
}

template<class Value>
void HashTable<Key, Value>::privateRehash(){
  #if test
    clock_gettime(CLOCK_REALTIME, &rehashStart);
  #endif
  size_t old_capacity = capacity;
  capacity = capacity << 1;
  Bucket<Key,Value>* temp = new Bucket<Key,Value>[capacity];
  HashNode<Key,Value>* node;
  HashNode<Key,Value>* next;
  for(size_t i = 0; i < old_capacity; i++){
    node = buckets[i].getNode();
    while(node != nullptr){
      next = node->getNext();
      temp[hash_func(node->getKey())].append(node);
      node = next;
    }
  }
  delete [] buckets;
  buckets = temp;
  #if test
    clock_gettime(CLOCK_REALTIME, &rehashEnd);
    rehashSum.tv_nsec = rehashSum.tv_nsec + rehashEnd.tv_nsec -rehashStart.tv_nsec;
  #endif
}

template<class Value>
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

template<class Value>
bool HashTable<Key, Value>::containsKey(const Key key){
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
  Bucket<Key,Value>* bucket = &buckets[hash_func(key)];
  auto node = bucket->getNode();
  std::shared_lock<std::shared_timed_mutex> lock(*(bucket->getMutex()));
  while (node != nullptr){
    if(node->getKey() == key){
      return true;
    }
    node = node->getNext();
  }
  return false;
}

template<class Value>
bool HashTable<Key, Value>::contains(const Value value){
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);

  for(size_t i = 0; i < load; i++){
      auto node = buckets[i].getNode();
      std::shared_lock<std::shared_timed_mutex> lock(*(node->getMutex()));
      while (node != nullptr){
        if(node->getValue() == value){
          return true;
        }
        node = node->getNext();
      }
  }

  return false;
}

template<class Value>
void HashTable<Key,Value>::rehash(){
  rehash_mutex.lock();
  privateRehash();
  rehash_mutex.unlock();
}

template<class Value>
size_t HashTable<Key,Value>::size(){
  return load;
}

template<class Value>
size_t HashTable<Key,Value>::getCapacity(){
  return capacity;
}

template<class Value>
void HashTable<Key,Value>::print(){
  unique_lock<shared_timed_mutex> hash_lock(rehash_mutex);
  for(size_t i = 0; i < capacity; i++){
    cout << "index : " << i << endl;
    auto node = buckets[i].getNode();
    while(node != nullptr){
      cout << "Key: " << node->getKey() << " Value: " << node->getValue() << endl;
      node = node->getNext();
    }
  }
}



//wtf??????
template class HashTable<int,int>;
template class HashTable<string,int>;
template class HashTable<int,string>;
template class HashTable<string,string>;
