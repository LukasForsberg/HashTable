#include "HashTable.h"
#include <iostream>

int main(){
  HashTable table = new HashTable();
  table.singleWrite("Edvin", 0821);
  table.singleWrite("Lukas", 0534);
  cout << "Edvin : "<< table.singleRead("Edvin") << endl;
  cout << "Lukas : << " << table.singleRead("Lukas") << endl;
}
