#include "HashNode.h"


template<class Key, class Value>
HashNode<Key, Value>::HashNode(Key k, Value val){
  key = k;
  value = val;
  next =4 nullptr;
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
HashNode<Key, Value> HashNode<Key, Value>::getNext(HashNode<Key, Value>* next_node){
  return next;
}
