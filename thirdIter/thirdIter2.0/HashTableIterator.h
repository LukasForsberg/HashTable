#ifndef HASHTABLE_ITERATOR_H
#define HASHTABLE_ITERATOR_H

#include <iterator>
#include "HashTable.h"

template<class Key, class Value> class HashTable;

template<class Key, class Value>
class HashTableIterator : public std::iterator<std::forward_iterator_tag, bool> {
public:

	HashTableIterator();
	HashTableIterator(HashTable<Key,Value>* table);

	bool operator!=(const HashTableIterator& hti) const;

	HashTableIterator& operator++();

	HashNode<Key,Value> operator*();

  HashTableIterator& operator=(const HashTableIterator& hti);

private:
	HashNode<Key,Value>* node;
  Bucket<Key,Value>* buckets;
  size_t index;
  size_t max_index;
};

#endif
