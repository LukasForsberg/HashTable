#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <cstdlib>
#include "InvalidReadException.h"
#include "HashNode.h"
#include "Bucket.h"
#include <thread>
#include <atomic>
#include "InvalidSizeException.h"
#include <sys/sysinfo.h>
#include "HashTableIterator.h"
#include <vector>
#include "../Arena/Arena.h"
#include <type_traits>

using namespace std;

template<class Key, class Value> class HashTableIterator;
template<class Key, class Value> class HashTable;

template<class Key, class Value>
struct arg_struct {
    thread t;
    Bucket<Key,Value>* temp;
    HashTable<Key,Value>* table;
    int index;
    size_t chunkSize;
};

#define test (0)

template<class Key, class Value> class HashTable {
  friend class HashTableIterator<Key,Value>;

  private:

    //attributes
    Bucket<Key,Value>* buckets;
    Arena<Key,Value>* arena;
    struct arg_struct<Key,Value>* thread_args;
    size_t capacity;
    atomic<uint16_t> load;
    const size_t cores = get_nprocs_conf();
    mutable shared_timed_mutex rehash_mutex;
    atomic_flag rehash_flag = ATOMIC_FLAG_INIT;

    //help functions
    void privateRehash(){
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

    static void* subHash(void *arguments){
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
            // assume that to prev different index can't be mapped to same index.
            temp[table->hash_func(node->getKey())].append(node);
            node = next;
        }
      }
      return nullptr;
    }

    bool isPowerOfTwo (int x){
        return x && (!(x&(x-1)));
    }

  public:
    HashTable(size_t size){
      if(!isPowerOfTwo(size)){
        throw InvalidSizeException();
      }
      buckets = new Bucket<Key,Value>[size];
      arena = new Arena<Key,Value>(size);
      thread_args = new struct arg_struct<Key,Value>[cores];
      capacity = size;
      load = 0;
    }

    ~HashTable(){
      unique_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
      delete [] buckets;
      delete arena;
      delete [] thread_args;
    }

    HashTable(const HashTable<Key, Value> &table){
      unique_lock<std::shared_timed_mutex> hash_lock(table.rehash_mutex);
      buckets = new Bucket<Key,Value>[table.capacity];
      arena = new Arena<Key,Value>(table.capacity);
      thread_args = new struct arg_struct<Key,Value>[cores];
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

    void write(Key key, Value value){
      {
        shared_lock<shared_timed_mutex> hash_lock(rehash_mutex);
        auto bucket = &buckets[hash_func(key)];
        HashNode<Key,Value>* node;
	      atomic_noexcept{
          node = bucket->getNode();
        }
        while(node != nullptr){
          atomic_noexcept{
            if(key == node->getKey()){
               node->setValue(value);
               break;
            }
            node = node->getNext();
          }
        }

        if(node == nullptr){
          node = arena->alloc(key,value);
          atomic_noexcept{
            bucket->append(node);
          }
          load.fetch_add(1, std::memory_order_relaxed);
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

    Value read(Key key){
      shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
      auto bucket = &buckets[hash_func(key)];
      HashNode<Key,Value>* node;

      atomic_noexcept{
        node = bucket->getNode();
      }
      while(node != nullptr){
        atomic_noexcept{
          if(key == node->getKey()){
            return node->getValue();
            break;
          }
          node = node->getNext();
        }
      }
      throw InvalidReadExeption();
    }

    Value readAndWrite(Key key, Value new_val){
      shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
      auto bucket = &buckets[hash_func(key)];
      HashNode<Key,Value>* node;
      Value v;
      atomic_noexcept{
        node = bucket->getNode();
      }
      while(node != nullptr){
        atomic_noexcept{
          if(key == node->getKey()){
            v = node->getValue();
            node->setValue(new_val);
            return v;
          }
          node = node->getNext();
        }
      }
      throw InvalidReadExeption();
    }

    size_t size(){
      return load;
    }

    size_t getCapacity(){
      return capacity;
    }

    size_t hash_func(Key key){
      hash<Key> h;
      return (h(key) % capacity);
    }

    bool remove(Key key){
      shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
      Bucket<Key,Value>* bucket = &buckets[hash_func(key)];
      HashNode<Key,Value>* node;
      HashNode<Key,Value>* prev_node = nullptr;
      bool del_flag = false;

      atomic_noexcept{
        node = bucket->getNode();
      }
      while(node != nullptr){
        atomic_noexcept{
          if(node->getKey() == key){
            if(prev_node != nullptr){
              prev_node->insertNext(node->getNext());
            } else {
              bucket->setNode(node->getNext());
            }
            del_flag = true;
            break;
          }
          prev_node = node;
          node = node->getNext();
        }
      }

      if(del_flag){
        load.fetch_sub(1, std::memory_order_relaxed);
        arena->free(node);
        return true;
      }
      return false;
    }

    bool contains(const Value value){
      shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
      Bucket<Key,Value>* bucket;
      HashNode<Key,Value>* node;
      for(size_t i = 0; i < capacity; i++){
        bucket = &buckets[i];
        //synchronized {
        atomic_noexcept{
          node = bucket->getNode();
        }
        while (node != nullptr){
          atomic_noexcept{
            if(node->getValue() == value){
              return true;
            }
            node = node->getNext();
          }
        }
      }
      return false;
    }

    vector<Key> getKeys(const Value value){
      shared_lock<shared_timed_mutex> hash_lock(rehash_mutex);
      vector<Key> v;
      bool cond;
      Key key;
      Bucket<Key,Value>* bucket;
      HashNode<Key,Value>* node;
      for(size_t i = 0; i < capacity; i++){
        bucket = &buckets[i];
        atomic_noexcept{
          node = bucket->getNode();
        }
        while (node != nullptr){
          atomic_noexcept{
            cond = (node->getValue() == value);
            key =  node->getKey();
            node = node->getNext();
          }
          if(cond){
            v.push_back(key);
          }
        }
      }
      return v;
    }

    bool containsKey(const Key key){
      shared_lock<std::shared_timed_mutex> hash_lock(rehash_mutex);
      Bucket<Key,Value>* bucket = &buckets[hash_func(key)];
      HashNode<Key,Value>* node;
      atomic_noexcept{
        node = bucket->getNode();
      }
      while (node != nullptr){
        atomic_noexcept{
          if(node->getKey() == key){
            return true;
          }
          node = node->getNext();
        }
      }
      return false;
    }

    void print(){
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

    bool empty(){
      return load == 0;
    }

    void rehash(){
      rehash_mutex.lock();
      privateRehash();
      rehash_mutex.unlock();
    }

    HashTableIterator<Key,Value> begin(){
      return HashTableIterator<Key,Value>(this);
    }

	  HashTableIterator<Key,Value> end(){
      return HashTableIterator<Key,Value>();
    }

    #if test
      struct timespec readStart, readEnd, readSum,
        writeStart, writeEnd,writeSum ,funcStart, funcEnd, funcSum,
        rehashStart, rehashEnd, rehashSum, memoryStart, memoryEnd, memorySum,
        hashStart, hashEnd, hashSum;
    #endif


};



#endif
