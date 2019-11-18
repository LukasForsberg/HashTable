#ifndef ARENA_H
#define ARENA_H

#include "SubArena.h"

template <typename T> class Arena {

private:
  //Size of the arenas created by the pool.
  size_t arena_size;
  // Current arena. Changes when it becomes full and we want to allocate one
  // more object.
  std::unique_ptr<SubArena<T>> arena;
  // List of free elements. The list can be threaded between different arenas
  // depending on the deallocation pattern.
  ArenaItem<T> *free_list;

public:
  // Creates a new pool that will use arenas of arena_size.
Arena(size_t arena_size)
    : arena_size(arena_size), arena(new SubArena<T>(arena_size)),
      free_list(arena->get_storage()) {}

// Allocates an object in the current arena.
template <typename... Args> T *alloc(Args &&... args) {
  if (free_list == nullptr) {
    // If the current arena is full, create a new one.
    std::unique_ptr<SubArena<T>> new_arena(new SubArena<T>(arena_size));
    // Link the new arena to the current one.
    new_arena->set_next_arena(std::move(arena));
    // Make the new arena the current one.
    arena.reset(new_arena.release());
    // Update the free_list with the storage of the just created arena.
    free_list = arena->get_storage();
  }

  // Get the first free item.
  ArenaItem<T> *current_item = free_list;
  // Update the free list to the next free item.
  free_list = current_item->get_next_item();

  // Get the storage for T.
  T *result = current_item->get_storage();
  // Construct the object in the obtained storage.
  new (result) T(std::forward<Args>(args)...);

  return result;
}

void free(T *t) {
  // Destroy the object.
  t->T::~T();

  // Convert this pointer to T to its enclosing pointer of an item of the
  // arena.
  ArenaItem<T> *current_item = ArenaItem<T>::storage_to_item(t);

  // Add the item at the beginning of the free list.
  current_item->set_next_item(free_list);
  free_list = current_item;
}
}; // minipool<T>

#endif
