#ifndef BUCKET_H
#define BUCKET_T

#include <mutex>
#include <shared_mutex>
#include <string>
#include "HashNode.h"

template<class Value> class Bucket{

public:
  void setNode(HashNode<Value>* newNode){ node = newNode; }
  HashNode<Value>* getNode(){ return node; }
  std::shared_timed_mutex* getMutex(){ return &mtx; }
  bool empty(){ return node == nullptr; };
  void append(HashNode<Value>* new_node);

  Bucket() { node = nullptr; }
  Bucket(HashNode<Value>* newNode) { node = newNode; }
  ~Bucket();
  Bucket(Bucket<Value> const& copy){ node = copy.node; }
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
  mutable std::shared_timed_mutex mtx;
  HashNode<Value>* node;
};


template class Bucket<int>;
template class Bucket<std::string>;

#endif
