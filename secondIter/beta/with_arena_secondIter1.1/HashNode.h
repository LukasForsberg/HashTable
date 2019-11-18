#ifndef HASHNODE_H
#define HASHNODE_H


template<class Key, class Value> class HashNode{

public:

  HashNode(Key key, Value value);
  HashNode();
  Key getKey();
  Value getValue();
  void insertNext(HashNode<Key, Value>* next_node);
  void setValue(Value &value);
  void setNode(Key key, Value value);
  HashNode<Key, Value>* getNext();

private:

  HashNode<Key, Value>* next;
  Key key;
  Value value;

};

#endif
