#ifndef HASHNODE_H
#define HASHNODE_H

#include <memory>
#include "custom/tm_string.h"
#include <thread>

template<class Key, class Value> class HashNode{

public:

  HashNode(Key key, Value value);
  HashNode();
  Key& getKey() transaction_safe;
  Value& getValue() transaction_safe;
  void insertNext(HashNode<Key, Value>* next_node)  transaction_safe;
  void setValue(Value value) transaction_safe;
  HashNode<Key, Value>* getNext()  transaction_safe;
  void setNode(Key key, Value value);

private:

  HashNode<Key, Value>* next;
  Key key;
  Value value;

};

#endif
