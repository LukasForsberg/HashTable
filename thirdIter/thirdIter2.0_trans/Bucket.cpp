#include "Bucket.h"
#include <iostream>

template<class Value>
void Bucket<Value>::append(HashNode<Value>* new_node){
    new_node->insertNext(node);
    node = new_node;
}

template<class Value>
Bucket<Value>::~Bucket() {

}
