#ifndef HASHNODE_H
#define HASHNODE_H

#include <memory>
#include <thread>

template<class Value> class HashNode{

public:

  HashNode(uint64_t key, Value value);
  Key getKey() transaction_safe ;
  Value getValue() transaction_safe ;
  void insertNext(HashNode<Value>* next_node);
  void setValue(Value value) transaction_safe;
  HashNode<Key, Value>* getNext() transaction_safe ;

private:

  HashNode<Value>* next;
  uint64_t key;
  Value value;

};

#endif
