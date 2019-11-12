

template<class Key, class Value> class HashNode{

public:

  HashNode(Key &key, Value &value);
  Key getKey()
  Value getValue();
  void insertNext(HashNode<Key, Value>* next_node);
  void setValue();
  HashNode<Key, Value>* getNext();

private:

  HashNode<Key, Value>* next;
  Key key;
  Value value;



}
