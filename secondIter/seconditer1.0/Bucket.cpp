#include "Bucket.h"
#include <iostream>

template<class Key, class Value>
void Bucket<Key,Value>::append(HashNode<Key, Value>* new_node){
    new_node->insertNext(node);
    node = new_node;
}

template<class Key, class Value>
Bucket<Key,Value>::~Bucket() {
  /*HashNode<Key,Value>* temp;
  while( node != nullptr){
    temp = node->getNext();
    std::cout << "deleting " << node->getKey() << "  " << node->getValue() << std::endl;
    delete node;
    node = temp;
  }*/
}
