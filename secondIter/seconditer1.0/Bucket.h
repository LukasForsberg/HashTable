#ifndef BUCKET_H
#define BUCKET_T

#include <mutex>
#include <shared_mutex>

template<class Key, class Value> class Bucket{

public:
  void setNode(HashNode<Key,Value>* newNode){ node = newNode }
  HashNode<Key,Value>* getNode(){ return node }
  std::shared_mutex mtx* getMutex(){ return &mtx }

  Bucket() { node = nullptr; }
  Bucket(HashNode<Key,Value>* newNode) { node = newNode; }
  ~Bucket() {/* should delete all nodes mabye??? */}
  Bucket(Bucket<Key,Value> const& copy){
    node = copy.node;
  }
  Bucket& operator=(Bucket rhs) // Pass by value (thus generating a copy)
  {
    this->node = rhs.node;
  }
  Bucket(Bucket&& src) noexcept{
    this->node = src.node;
  }
  Bucket& operator=(Bucket&& src) noexcept{
     this->node = src.node;
     return *this
  }


private:
  mutable std::shared_mutex mtx;
  HashNode<Key,Value>* node;
};

#endif
