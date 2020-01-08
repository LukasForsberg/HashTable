#ifndef ARENAITEM_H
#define ARENAITEM_H

#include <memory>
#include "../Table/HashTable.h"

template<class Key, class Value>  class ArenaItem {
private:

    HashNode<Key, Value> datum;
    // Points to the next freely available item.
    ArenaItem *next;
    // Storage of the item. Note that this is a union
    // so it is shared with the pointer "next" above.


  public:
  // Methods for the list of free items.
  ArenaItem *get_next_item() const { return next; }
  void set_next_item(ArenaItem *n) { next = n; }

  // Methods for the storage of the item.
  HashNode<Key, Value> *get_storage() { return &datum; }

  static ArenaItem<Key,Value> *storage_to_item(HashNode<Key,Value> *t) {
      ArenaItem<Key,Value> *current_item = reinterpret_cast<ArenaItem<Key,Value> *>(t);
      return current_item;
    }

}; // minipool_item

#endif
