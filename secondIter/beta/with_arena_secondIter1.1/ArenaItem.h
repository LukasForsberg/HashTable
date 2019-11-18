#ifndef ARENAITEM_H
#define ARENAITEM_H

#include <memory>

template <typename T> union ArenaItem {
  private:
    using StorageType = alignas(alignof(T)) char[sizeof(T)];

    // Points to the next freely available item.
    ArenaItem *next;
    // Storage of the item. Note that this is a union
    // so it is shared with the pointer "next" above.
    StorageType datum;

  public:
  // Methods for the list of free items.
  ArenaItem *get_next_item() const { return next; }
  void set_next_item(ArenaItem *n) { next = n; }

  // Methods for the storage of the item.
  T *get_storage() { return reinterpret_cast<T *>(datum); }

  // Given a T* cast it to a minipool_item*
  static ArenaItem *storage_to_item(T *t) {
    ArenaItem *current_item = reinterpret_cast<ArenaItem *>(t);
    return current_item;
  }
}; // minipool_item

#endif
