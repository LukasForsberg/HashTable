#include "HashTableIterator.h"

template<class Value>
HashTableIterator<Value>::HashTableIterator(HashTable<Value>* table){
  this->buckets = table->buckets;
  node = this->buckets[0].getNode();
  index = 0;
  max_index = table->getCapacity();
}

template<class Value>
HashTableIterator<Value>::HashTableIterator(){
  buckets = nullptr;
  node = nullptr;
  index = 0;
  max_index = 0;
}

template<class Value>
bool HashTableIterator<Value>::operator!=(const HashTableIterator<Value>& hti) const {
  return this->node != hti.node;
}

template<class Value>
HashTableIterator<Value>& HashTableIterator<Value>::operator++() {
  auto new_node = this->node->getNext();
  while(new_node == nullptr && index < max_index){
    index++;
    new_node = buckets[index].getNode();
  }
  this->node = new_node;
  return *this;
}

template<class Value>
HashNode<Value> HashTableIterator<Value>::operator*() {
  return *node;
}

template<class Value>
HashTableIterator<Value>& HashTableIterator<Value>::operator=(const HashTableIterator<Value>& hti) {
  this->node = hti.node;
  this->buckets = hti.buckets;
  this->index = hti.index;
  this->max_index = hti.max_index;
  return *this;
}

//wtf??????
template class HashTableIterator<uint64_t>;
template class HashTableIterator<int>;
