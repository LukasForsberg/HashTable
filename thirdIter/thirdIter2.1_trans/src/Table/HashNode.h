#ifndef HASHNODE_H
#define HASHNODE_H

#include <memory>
#include "custom/tm_string.h"
#include <thread>

template<class Key, class Value> class HashNode{

public:

  HashNode(Key key, Value value){
    this->key = std::move(key);
    this->value = std::move(value);
    next = nullptr;
  }

  HashNode(){
    next = nullptr;
  }

  Key& getKey() transaction_safe{
    return key;
  }

  Value& getValue() transaction_safe{
    return value;
  }

  void insertNext(HashNode<Key, Value>* next_node)  transaction_safe{
    next = next_node;
  }

  void setValue(Value value) transaction_safe{
    this->value = std::move(value);
  }

  HashNode<Key, Value>* getNext()  transaction_safe{
    return next;
  }

  void setNode(Key key, Value value){
    this->key = std::move(key);
    this->value = std::move(value);
  }

private:

  HashNode<Key, Value>* next;
  Key key;
  Value value;

};

#endif
