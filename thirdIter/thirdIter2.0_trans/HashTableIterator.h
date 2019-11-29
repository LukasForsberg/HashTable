#ifndef HASHTABLE_ITERATOR_H
#define HASHTABLE_ITERATOR_H

#include <iterator>
#include "HashTable.h"

template<class Value> class HashTable;

template<class Value>
class HashTableIterator : public std::iterator<std::forward_iterator_tag, bool> {
public:

	HashTableIterator();
	HashTableIterator(HashTable<Value>* table);

	bool operator!=(const HashTableIterator& hti) const;

	HashTableIterator& operator++();

	HashNode<Key,Value> operator*();

  HashTableIterator& operator=(const HashTableIterator& hti);

private:
	HashNode<Value>* node;
  Bucket<Value>* buckets;
  size_t index;
  size_t max_index;
};

#endif
