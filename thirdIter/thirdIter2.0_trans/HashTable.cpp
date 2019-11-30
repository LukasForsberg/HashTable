#include "HashTable.h"
#include <type_traits>
#include <sys/sysinfo.h>

bool isPowerOfTwo (uint64_t x){
    return x && (!(x&(x-1)));
}

template<class Value>
HashTable<Value>::HashTable(uint64_t size){
  if(!isPowerOfTwo(size)){
    throw InvalidSizeException();
  }
  buckets = new Bucket<Value>[size];
  capacity = size;
  load = 0;
}

template<class Value>
HashTable<Value>::~HashTable(){
  unique_lock<shared_timed_mutex> hash_lock(rehash_mutex);
  HashNode<Value>* node;
  HashNode<Value>* temp;
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
HashTable<Value>::HashTable(const HashTable<Value> &table){
  unique_lock<std::shared_timed_mutex> hash_lock(table.rehash_mutex);
  buckets = new Bucket<Value>[table.capacity];
  load = table.load.load(memory_order_relaxed);
  capacity = table.capacity;

  for(size_t i = 0; i < capacity; i++){
    auto node = table.buckets[i].getNode();
    while(node != nullptr){
      HashNode<Value>* freshNode = new HashNode<Value>(node->getKey(), node->getValue());
      buckets[i].append(freshNode);
      node = node->getNext();
    }
  }
}

template<class Value>
void HashTable<Value>::singleWrite(uint64_t key, Value value){
    {
    shared_lock<shared_timed_mutex> hash_lock(rehash_mutex);
    auto bucket = &buckets[hash_func(key)];
    HashNode<Value>* node;
    synchronized{
    // atomic_noexcept{
      node = bucket->getNode();
      while(node != nullptr){
        if(key == node->getKey()){
           node->setValue(value);
           break;
        }
        node = node->getNext();
      }
    }
    if(node == nullptr){
      node = new HashNode<Value>(key,value);
      synchronized{
        bucket->append(node);
      }
      load.fetch_add(1, std::memory_order_relaxed);
    }
  }

  // equal to load/capacity > 0.625
  // idea make a specific thread rehash.
  if(load.load(memory_order_relaxed) > ( (capacity >> 1) + (capacity >> 2) - (capacity >> 3) ) ){
    if(!rehash_flag.test_and_set(memory_order_relaxed)){
      rehash_mutex.lock();
      privateRehash();
      rehash_mutex.unlock();
      rehash_flag.clear();
    }
  }
}

template<class Value>
bool HashTable<Value>::readAndWrite(uint64_t key, Value new_val, Value old_val){
  shared_lock<shared_timed_mutex> hash_lock(rehash_mutex);
  auto bucket = &buckets[hash_func(key)];
  auto node = bucket->getNode();

  synchronized {
  // atomic_noexcept{
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
  }
  return false;
}

template<class Value>
bool equals(uint64_t k1, uint64_t k2) transaction_safe {
  return k1 == k2;
}

template<class Value>
Value HashTable<Value>::singleRead(uint64_t key){
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
  auto bucket = &buckets[hash_func(key)];
  auto node = bucket->getNode();

  //synchronized {
  atomic_noexcept{
    while(node != nullptr){
      if(key == node->getKey()){
        return node->getValue();
      }
      node = node->getNext();
    }
  }
  throw InvalidReadExeption();
}


template<class Value>
size_t HashTable<Value>::hash_func(uint64_t key){
  hash<uint64_t> h;
  return (h(key) % capacity);
}

template<class Value>
struct arg_struct {
    int index;
    Bucket<Value>* temp;
    HashTable<Value>* table;
};

template<class Value>
void HashTable<Value>::privateRehash(){
  size_t no_threads = capacity >> 6;
  capacity = capacity << 1;
  Bucket<Value>* temp = new Bucket<Value>[capacity];

  thread* helpThreads = new thread[no_threads];
  struct arg_struct<Value>* args = new struct arg_struct<Value>[no_threads];

  for( size_t i = 0; i < no_threads; i++ ){
    args[i].index = i;
    args[i].temp = temp;
    args[i].table = this;
    helpThreads[i] = thread(HashTable<Value>::subHash, &args[i]);
  }

  for (size_t i = 0; i < no_threads; i++){
      helpThreads[i].join();
 }

  delete [] buckets;
  delete [] helpThreads;
  delete [] args;
  buckets = temp;
}

template<class Value>
bool HashTable<Value>::remove(uint64_t key){
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
  Bucket<Value>* bucket = &buckets[hash_func(key)];
  HashNode<Value>* node;
  HashNode<Value>* prev_node = nullptr;
  bool del_flag = false;

  synchronized {
    // atomic_noexcept{
    node = bucket->getNode();
    while(node != nullptr){
      if(node->getKey() == key){
        if(prev_node != nullptr){
          prev_node->insertNext(node->getNext());
        } else {
          bucket->setNode(node->getNext());
        }
        load.fetch_sub(1, std::memory_order_relaxed);
        del_flag = true;
        break;
      }
      prev_node = node;
      node = node->getNext();
    }
  }
  if(del_flag){
    delete node;
    return true;
  }
  return false;
}

template<class Value>
bool HashTable<Value>::containsKey(const uint64_t key){
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
  Bucket<Value>* bucket = &buckets[hash_func(key)];
  synchronized {
  // atomic_noexcept{
    auto node = bucket->getNode();
    while (node != nullptr){
      if(node->getKey() == key){
        return true;
      }
      node = node->getNext();
    }
  }
  return false;
}

template<class Value>
bool HashTable<Value>::contains(const Value value){
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
  Bucket<Value>* bucket;
  HashNode<Value>* node;
  for(size_t i = 0; i < capacity; i++){
    bucket = &buckets[i];
    synchronized {
    // atomic_noexcept{
      node = bucket->getNode();
      while (node != nullptr){
        if(node->getValue() == value){
          return true;
        }
        node = node->getNext();
      }
    }
  }
  return false;
}

template<class Value>
vector<uint64_t> HashTable<Value>::getKeys(const Value value){
  shared_lock<shared_timed_mutex> hash_lock(rehash_mutex);
  vector<uint64_t> v;
  Bucket<Value>* bucket;
  HashNode<Value>* node;
  for(size_t i = 0; i < capacity; i++){
    bucket = &buckets[i];
    synchronized {
    // atomic_noexcept{
      node = bucket->getNode();
      while (node != nullptr){
        if(node->getValue() == value){
          v.push_back(node->getKey());
        }
        node = node->getNext();
      }
    }
  }
  return v;
}

template<class Value>
void HashTable<Value>::rehash(){
  while(rehash_flag.test_and_set(memory_order_relaxed));
  rehash_mutex.lock();
  privateRehash();
  rehash_mutex.unlock();
  rehash_flag.clear();
}

template<class Value>
size_t HashTable<Value>::size(){
  return load;
}

template<class Value>
size_t HashTable<Value>::getCapacity(){
  return capacity;
}

template<class Value>
bool HashTable<Value>::empty(){
  return load == 0;
}

template<class Value>
void HashTable<Value>::print(){
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

template<class Value>
void* HashTable<Value>::subHash(void *arguments){
  struct arg_struct<Value> *args = (arg_struct<Value>*)arguments;
  size_t index = args->index;
  Bucket<Value>* temp = args-> temp;
  HashNode<Value>* node;
  HashNode<Value>* next;
  HashTable<Value>* table = args->table;

  for(size_t i = 64*index; i < 64*(index + 1) ; i++){

    node = table->buckets[i].getNode();

      while(node != nullptr){
        next = node->getNext();
        temp[table->hash_func(node->getKey())].append(node);
        node = next;
    }
  }
  return nullptr;
}

template<class Value>
HashTableIterator<Value> HashTable<Value>::begin() {
	return HashTableIterator<Value>(this);
}

template<class Value>
HashTableIterator<Value> HashTable<Value>::end() {
	return HashTableIterator<Value>();
}



//wtf??????
template class HashTable<uint64_t>;
template class HashTable<int>;
