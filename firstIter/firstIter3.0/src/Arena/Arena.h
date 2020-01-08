#ifndef ARENA_H
#define ARENA_H

#include "SubArena.h"
#include <iostream>

template<class Key, class Value> class Arena {

private:
  //Size of the arenas created by the pool.
  size_t arena_size;
  // Current arena. Changes when it becomes full and we want to allocate one
  // more object.
  SubArena<Key,Value>* arena;
  // List of free elements. The list can be threaded between different arenas
  // depending on the deallocation pattern.
  ArenaItem<Key,Value> *free_list;


public:
  // Creates a new pool that will use arenas of arena_size.
Arena(size_t arena_size)
    : arena_size(arena_size), arena(new SubArena<Key,Value>(arena_size)),
      free_list(arena->get_storage()) {}

~Arena(){
  SubArena<Key,Value>* next;
  do{
    next = arena->get_next();
    delete arena;
    arena = next;
  }while(arena != nullptr);
}

// Allocates an object in the current arena.
HashNode<Key,Value>* alloc(Key key, Value value) {
  if (free_list == nullptr) {
    // If the current arena is full, create a new one.
    SubArena<Key,Value>* new_arena = new SubArena<Key,Value>(arena_size);
    // Link the new arena to the current one.
    new_arena->set_next_arena(arena);
    // Make the new arena the current one.
    arena = new_arena;
    // Update the free_list with the storage of the just created arena.
    free_list = arena->get_storage();
  }
  // Get the first free item.
  ArenaItem<Key,Value> *current_item = free_list;
  // Update the free list to the next free item.
  free_list = current_item->get_next_item();
  // Get the storage for T.
  HashNode<Key,Value> *result = current_item->get_storage();
  // Construct the object in the obtained storage.
  result->setNode(key,value);
  return result;
}

void free(HashNode<Key,Value> *t) {
  // Convert this pointer to T to its enclosing pointer of an item of the
  // arena.
  ArenaItem<Key,Value> *current_item = ArenaItem<Key,Value>::storage_to_item(t);

  // Add the item at the beginning of the free list.
  current_item->set_next_item(free_list);
  free_list = current_item;
}

void setSize(size_t s){
  arena_size = s;
}

};

#endif
