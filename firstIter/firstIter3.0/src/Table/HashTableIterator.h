#ifndef HASHTABLE_ITERATOR_H
#define HASHTABLE_ITERATOR_H

#include <iterator>
#include "HashTable.h"

template<class Key, class Value> class HashTable;

template<class Key, class Value>
class HashTableIterator : public std::iterator<std::forward_iterator_tag, bool> {
public:

	HashTableIterator(){
		buckets = nullptr;
		node = nullptr;
		index = 0;
		max_index = 0;
	}

	HashTableIterator(HashTable<Key,Value>* table){
		this->buckets = table->buckets;
	  node = this->buckets[0].getNode();
	  index = 0;
	  max_index = table->getCapacity();
	}

	bool operator!=(const HashTableIterator& hti) const{
		return this->node != hti.node;
	}

	HashTableIterator& operator++(){
		auto new_node = this->node->getNext();
		while(new_node == nullptr && index < max_index){
			index++;
			new_node = buckets[index].getNode();
		}
		this->node = new_node;
		return *this;
	}

	HashNode<Key,Value> operator*(){
		return *node;
	}

  HashTableIterator& operator=(const HashTableIterator& hti){
		this->node = hti.node;
		this->buckets = hti.buckets;
		this->index = hti.index;
		this->max_index = hti.max_index;
		return *this;
	}

private:
	HashNode<Key,Value>* node;
  Bucket<Key,Value>* buckets;
  size_t index;
  size_t max_index;
};

#endif
