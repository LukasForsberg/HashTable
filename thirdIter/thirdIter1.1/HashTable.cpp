#include "HashTable.h"
#include <type_traits>
#include <sys/sysinfo.h>

bool isPowerOfTwo (int x){
    return x && (!(x&(x-1)));
}

template<class Key, class Value>
HashTable<Key, Value>::HashTable(size_t size){
  if(!isPowerOfTwo(size)){
    throw InvalidSizeException();
  }
  buckets = new Bucket<Key,Value>[size];
  arena = new Arena<Key,Value>(size);
  thread_args = new struct arg_struct<Key,Value>[cores];
  capacity = size;
  load = 0;
}

template<class Key, class Value>
HashTable<Key, Value>::~HashTable(){
  unique_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
  delete [] buckets;
  delete arena;
  delete [] thread_args;
}

template<class Key, class Value>
HashTable<Key, Value>::HashTable(const HashTable<Key, Value> &table){
  unique_lock<std::shared_timed_mutex> hash_lock(table.rehash_mutex);
  buckets = new Bucket<Key,Value>[table.capacity];
  arena = new Arena<Key,Value>(table.capacity);
  load = table.load.load(memory_order_relaxed);
  capacity = table.capacity;

  for(size_t i = 0; i < capacity; i++){
    auto node = table.buckets[i].getNode();
    while(node != nullptr){
      HashNode<Key, Value>* freshNode = arena->alloc(node->getKey(),node->getValue());
      buckets[i].append(freshNode);
      node = node->getNext();
    }
  }
}

template<class Key, class Value>
void HashTable<Key, Value>::singleWrite(Key key, Value value){
  {
    shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
    auto bucket = &buckets[hash_func(key)];
    HashNode<Key,Value>* node;
    {
      unique_lock<shared_timed_mutex> lock(*(bucket->getMutex()));
      node = bucket->getNode();
      while(node != nullptr){
        if(key == node->getKey()){
           node->setValue(value);
           break;
        }
        node = node->getNext();
      }
      if(node == nullptr){
        node = arena->alloc(key,value);
        bucket->append(node);
        load.fetch_add(1, std::memory_order_relaxed);
      }
    }
  }

  // equal to load/capacity > 0.625
  if(load.load(memory_order_relaxed) > ( (capacity >> 1) + (capacity >> 2) - (capacity >> 3) ) ){
    if(!rehash_flag.test_and_set(memory_order_relaxed)){
      rehash_mutex.lock();
      privateRehash();
      rehash_mutex.unlock();
      rehash_flag.clear();
    }
  }
}

template<class Key, class Value>
bool HashTable<Key, Value>::readAndWrite(Key key, Value new_val, Value old_val){
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
  auto bucket = &buckets[hash_func(key)];
  HashNode<Key,Value>* node;
  {
    unique_lock<shared_timed_mutex> lock(*(bucket->getMutex()));
    node = bucket->getNode();
    while(node != nullptr){
      if(key == node->getKey()){
        if(node->getValue() == old_val){
          node->setValue(new_val);
          return true;
        }
        else {
          return false;
        }
      }
      node = node->getNext();
    }
    return false;
  }
}

template<class Key, class Value>
Value HashTable<Key, Value>::singleRead(Key key){
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
  auto bucket = &buckets[hash_func(key)];
  HashNode<Key,Value>* node;
  {
    shared_lock<shared_timed_mutex> lock(*(bucket->getMutex()));
    node = bucket->getNode();
    while(node != nullptr){
      if(key == node->getKey()){
        return node->getValue();
      }
      node = node->getNext();
    }
    throw InvalidReadExeption();
  }
}


template<class Key, class Value>
size_t HashTable<Key, Value>::hash_func(Key key){
  hash<Key> h;
  return (h(key) % capacity);
}

template<class Key, class Value>
void HashTable<Key, Value>::privateRehash(){

  size_t no_threads;
  size_t chunkSize;

  if(cores < (capacity >> 6)){
    no_threads = cores;
    chunkSize = capacity / cores;
  } else {
     if(capacity >= 64){
       chunkSize = 64;
       no_threads = capacity >> 6;
     } else {
       chunkSize = capacity;
       no_threads = 1;
     }
  }

  capacity = capacity << 1;
  arena->setSize(capacity);
  Bucket<Key,Value>* temp = new Bucket<Key,Value>[capacity];

  for( size_t i = 0; i < no_threads; i++ ) {
    thread_args[i].chunkSize = chunkSize;
    thread_args[i].index = i;
    thread_args[i].temp = temp;
    thread_args[i].table = this;
    thread_args[i].t = thread(HashTable<Key,Value>::subHash, &thread_args[i]);
  }

  for (size_t i = 0; i < no_threads; i++){
      thread_args[i].t.join();
 }

  delete [] buckets;
  buckets = temp;
}

template<class Key, class Value>
bool HashTable<Key, Value>::remove(Key key){
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
  auto index = hash_func(key);
  Bucket<Key,Value>* bucket = &buckets[index];
  HashNode<Key,Value>* node;
  HashNode<Key,Value>* prev_node = nullptr;
  {
    std::unique_lock<std::shared_timed_mutex> lock(*(bucket->getMutex()));
    node = bucket->getNode();
    while(node != nullptr){
      if(node->getKey() == key){
        if(prev_node != nullptr){
          prev_node->insertNext(node->getNext());
        } else {
          bucket->setNode(node->getNext());
        }
        load.fetch_sub(1, std::memory_order_relaxed);
        arena->free(node);
        return true;
      }
      prev_node = node;
      node = node->getNext();
    }
  }
  return false;
}

template<class Key, class Value>
bool HashTable<Key, Value>::containsKey(const Key key){
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
  Bucket<Key,Value>* bucket = &buckets[hash_func(key)];
  HashNode<Key,Value>* node;
  std::shared_lock<std::shared_timed_mutex> lock(*(bucket->getMutex()));
  node = bucket->getNode();
  while (node != nullptr){
    if(node->getKey() == key){
      return true;
    }
    node = node->getNext();
  }
  return false;
}

template<class Key, class Value>
bool HashTable<Key, Value>::contains(const Value value){
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
  HashNode<Key,Value>* node;
  for(size_t i = 0; i < capacity; i++){
    std::shared_lock<std::shared_timed_mutex> lock(*(buckets[i].getMutex()));
      node = buckets[i].getNode();
      while (node != nullptr){
        if(node->getValue() == value){
          return true;
        }
        node = node->getNext();
      }
  }
  return false;
}

template<class Key, class Value>
vector<Key> HashTable<Key, Value>::getKeys(const Value value){
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
  vector<Key> v;
  HashNode<Key,Value>* node;
  for(size_t i = 0; i < capacity; i++){
  shared_lock<std::shared_timed_mutex> lock(*(buckets[i].getMutex()));
      node = buckets[i].getNode();
      while (node != nullptr){
        if(node->getValue() == value){
          v.push_back(node->getKey());
        }
        node = node->getNext();
      }
  }
  return v;
}

template<class Key, class Value>
void HashTable<Key,Value>::rehash(){
  rehash_mutex.lock();
  privateRehash();
  rehash_mutex.unlock();
}

template<class Key, class Value>
size_t HashTable<Key,Value>::size(){
  return load;
}

template<class Key, class Value>
size_t HashTable<Key,Value>::getCapacity(){
  return capacity;
}

template<class Key, class Value>
bool HashTable<Key,Value>::empty(){
  return load == 0;
}

template<class Key, class Value>
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

template<class Key, class Value>
void* HashTable<Key,Value>::subHash(void *arguments){
  struct arg_struct<Key,Value> *args = (arg_struct<Key,Value>*)arguments;
  size_t index = args->index;
  size_t chunkSize = args->chunkSize;
  Bucket<Key,Value>* temp = args-> temp;
  HashNode<Key,Value>* node;
  HashNode<Key,Value>* next;
  HashTable<Key,Value>* table = args->table;

  for(size_t i = chunkSize*index; i < chunkSize*(index + 1) ; i++){

    node = table->buckets[i].getNode();

      while(node != nullptr){
        next = node->getNext();
        temp[table->hash_func(node->getKey())].append(node);

        node = next;
    }
  }
  return nullptr;
}

template<class Key, class Value>
HashTableIterator<Key,Value> HashTable<Key,Value>::begin() {
	return HashTableIterator<Key,Value>(this);
}

template<class Key, class Value>
HashTableIterator<Key,Value> HashTable<Key,Value>::end() {
	return HashTableIterator<Key,Value>();
}



//wtf??????
template class HashTable<int,int>;
template class HashTable<string,int>;
template class HashTable<int,string>;
template class HashTable<string,string>;
