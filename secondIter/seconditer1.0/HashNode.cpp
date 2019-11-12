#include "HashNode.h"
#include <string>

using std::string;

template<class Key, class Value>
HashNode<Key, Value>::HashNode(Key& k, Value& val){
  key = k;
  value = val;
  next = nullptr;
}

template<class Key, class Value>
Key HashNode<Key, Value>::getKey(){
  return key;
}

template<class Key, class Value>
Value HashNode<Key, Value>::getValue(){
  return value;
}

template<class Key, class Value>
void HashNode<Key, Value>::setValue(Value &val){
  value = val;
}

template<class Key, class Value>
void HashNode<Key, Value>::insertNext(HashNode<Key, Value>* next_node){
  next = next_node;
}

template<class Key, class Value>
HashNode<Key, Value>* HashNode<Key, Value>::getNext(){
  return next;
}

template<class Key, class Value>
std::shared_mutex*  HashNode<Key, Value>::getMutex(){
  return &mtx;
}

template<class Key, class Value>
bool HashNode<Key, Value>::isUsed(){
  return used;
}

template<class Key, class Value>
void HashNode<Key, Value>::setUsed(bool flag){
  used = flag;
}

template class HashNode<int,int>;
template class HashNode<string,int>;
template class HashNode<int,string>;
template class HashNode<string,string>;
