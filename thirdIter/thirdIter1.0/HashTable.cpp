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
  capacity = size;
  load = 0;
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
HashTable<Key, Value>::HashTable(const HashTable<Key, Value> &table){

  buckets = new Bucket<Key,Value>[table.capacity];
  load = table.load.load(memory_order_relaxed);
  capacity = table.capacity;

  for(size_t i = 0; i < capacity; i++){

    auto node = table.buckets[i].getNode();

    while(node != nullptr){

      HashNode<Key, Value>* freshNode = new HashNode<Key, Value>(node->getKey(), node->getValue());
      buckets[i].append(freshNode);
      node = node->getNext();
    }
  }
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
      privateRehash();
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
  hash<Key> h;
  return (h(key) % capacity);
}

template<class Key, class Value>
struct arg_struct {
    int index;
    Bucket<Key,Value>* temp;
    HashTable<Key,Value>* table;
};

template<class Key, class Value>
void HashTable<Key, Value>::privateRehash(){
  size_t no_threads = capacity >> 6;
  capacity = capacity << 1;
  Bucket<Key,Value>* temp = new Bucket<Key,Value>[capacity];

  thread* helpThreads = new thread[no_threads];
  struct arg_struct<Key,Value>* args = new struct arg_struct<Key,Value>[no_threads];

  for( size_t i = 0; i < no_threads; i++ ) {
    args[i].index = i;
    args[i].temp = temp;
    args[i].table = this;
    helpThreads[i] = thread(HashTable<Key,Value>::subHash, &args[i]);
  }

  for (size_t i = 0; i < no_threads; i++){

      helpThreads[i].join();
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

template<class Key, class Value>
bool HashTable<Key, Value>::contains(const Value value){
  shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);

  for(size_t i = 0; i < capacity; i++){
    std::shared_lock<std::shared_timed_mutex> lock(*(buckets[i].getMutex()));
      auto node = buckets[i].getNode();
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
  struct arg_struct<Key, Value> *args = (arg_struct<Key,Value>*)arguments;
  size_t index = args->index;
  Bucket<Key,Value>* temp = args-> temp;
  HashNode<Key,Value>* node;
  HashNode<Key,Value>* next;
  HashTable<Key,Value>* table = args->table;

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
