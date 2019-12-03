#ifndef BUCKET_H
#define BUCKET_T

#include <mutex>
#include <shared_mutex>
#include <string>
#include "HashNode.h"
#include "custom/tm_string.h"

template<class Key, class Value> class Bucket{

public:
  void setNode(HashNode<Key,Value>* newNode){ node = newNode; }
  HashNode<Key,Value>* getNode(){ return node; }
  bool empty(){ return node == nullptr; };
  void append(HashNode<Key, Value>* new_node) transaction_safe{
    new_node->insertNext(node);
    node = new_node;
  }

  Bucket() { node = nullptr; }
  Bucket(HashNode<Key,Value>* newNode) { node = newNode; }
  Bucket(Bucket<Key,Value> const& copy){ node = copy.node; }
  Bucket& operator=(Bucket rhs) // Pass by value (thus generating a copy)
  {
    this->node = rhs.node;
    return *this;
  }
  Bucket(Bucket&& src) noexcept{
    this->node = src.node;
  }
  Bucket& operator=(Bucket&& src) noexcept{
     this->node = src.node;
     return *this;
  }


private:
  HashNode<Key,Value>* node;
};


template class Bucket<int,int>;
template class Bucket<tm_string,int>;
template class Bucket<int,tm_string>;
template class Bucket<tm_string,tm_string>;
#endif
