#include "HashTableIterator.h"

template<class Key, class Value>
HashTableIterator<Key,Value>::HashTableIterator(HashTable<Key,Value>* table){
  this->buckets = table->buckets;
  node = this->buckets[0].getNode();
  index = 0;
  max_index = table->getCapacity();
}

template<class Key, class Value>
HashTableIterator<Key,Value>::HashTableIterator(){
  buckets = nullptr;
  node = nullptr;
  index = 0;
  max_index = 0;
}

template<class Key, class Value>
bool HashTableIterator<Key,Value>::operator!=(const HashTableIterator<Key,Value>& hti) const {
  return this->node != hti.node;
}

template<class Key, class Value>
HashTableIterator<Key,Value>& HashTableIterator<Key,Value>::operator++() {
  auto new_node = this->node->getNext();
  while(new_node == nullptr && index < max_index){
    index++;
    new_node = buckets[index].getNode();
  }
  this->node = new_node;
  return *this;
}

template<class Key, class Value>
HashNode<Key,Value> HashTableIterator<Key,Value>::operator*() {
  return *node;
}

template<class Key, class Value>
HashTableIterator<Key,Value>& HashTableIterator<Key,Value>::operator=(const HashTableIterator<Key,Value>& hti) {
  this->node = hti.node;
  this->buckets = hti.buckets;
  this->index = hti.index;
  this->max_index = hti.max_index;
  return *this;
}

//wtf??????
template class HashTableIterator<int,int>;
template class HashTableIterator<string,int>;
template class HashTableIterator<int,string>;
template class HashTableIterator<string,string>;
