#include "HashNode.h"
#include <string>

using std::string;

template<class Value>
HashNode<Value>::HashNode(uint64_t k, Value val){
  key = std::move(k);
  value = std::move(val);
  next = nullptr;
}

template<class Value>
uint64_t HashNode<Value>::getKey(){
  return key;
}

template<class Value>
Value HashNode<Value>::getValue(){
  return value;
}

template<class Value>
void HashNode<Value>::setValue(Value val){
  value = std::move(val);
}

template<class Value>
void HashNode<Value>::insertNext(HashNode<Value>* next_node){
  next = next_node;
}

template<class Value>
HashNode<Value>* HashNode<Value>::getNext(){
  return next;
}

template class HashNode<uint64_t>;
template class HashNode<string>;
