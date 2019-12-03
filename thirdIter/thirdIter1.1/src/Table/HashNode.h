#ifndef HASHNODE_H
#define HASHNODE_H

#include <memory>

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

  Key getKey(){
    return key;
  }

  Value getValue(){
    return value;
  }

  void insertNext(HashNode<Key, Value>* next_node){
    next = next_node;
  }

  void setValue(Value value){
    this->value = std::move(value);
  }

  void setNode(Key key, Value value){
    this->key = std::move(key);
    this->value = std::move(value);
  }

  HashNode<Key, Value>* getNext(){
      return next;
  }

private:

  HashNode<Key, Value>* next;
  Key key;
  Value value;
};

#endif
