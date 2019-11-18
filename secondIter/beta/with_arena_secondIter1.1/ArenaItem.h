#ifndef ARENAITEM_H
#define ARENAITEM_H

#include <memory>
#include "HashTable.h"

template<class Key, class Value>  union ArenaItem {
private:

    // Points to the next freely available item.
    ArenaItem *next;
    // Storage of the item. Note that this is a union
    // so it is shared with the pointer "next" above.
    HashNode<Key, Value> datum;

  public:
  // Methods for the list of free items.
  ArenaItem *get_next_item() const { return next; }
  void set_next_item(ArenaItem *n) { next = n; }

  // Methods for the storage of the item.
  HashNode<Key, Value> *get_storage() { return datum; }

}; // minipool_item

#endif
