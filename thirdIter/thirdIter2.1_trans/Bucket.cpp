#include "Bucket.h"
#include <iostream>

template<class Key, class Value>
void Bucket<Key,Value>::append(HashNode<Key, Value>* new_node){
    new_node->insertNext(node);
    node = new_node;
}
