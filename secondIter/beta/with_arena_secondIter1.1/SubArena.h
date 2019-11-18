#ifndef SUBARENA_H
#define SUBARENA_H

#include "ArenaItem.h"
#include <cassert>

// Arena of items. This is just an array of items and a pointer
 // to another arena. All arenas are singly linked between them.
template<class Key, class Value> class SubArena {
 private:
   // Storage of this arena.
   ArenaItem<Key,Value>* storage;
   // Pointer to the next arena.
   SubArena<Key,Value>* next;

 public:
   // Creates an arena with arena_size items.
   SubArena(size_t arena_size) : storage(new ArenaItem<Key,Value>[arena_size]) {
    for (size_t i = 1; i < arena_size; i++) {
      storage[i - 1].set_next_item(&storage[i]);
    }
    storage[arena_size - 1].set_next_item(nullptr);
  }

  // Returns a pointer to the array of items. This is used by the arena
   // itself. This is only used to update free_list during initialization
   // or when creating a new arena when the current one is full.
   ArenaItem<Key,Value> *get_storage() const { return storage; }

   // Sets the next arena. Used when the current arena is full and
  // we have created this one to get more storage.
  void set_next_arena(SubArena<Key, Value>* n) {
    next = n;
  }
}; // minipool_arena

#endif
