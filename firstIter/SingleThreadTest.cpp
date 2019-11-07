#include "HashTable.h"
#include <iostream>
#include <string>



int main(){

  HashTable<string,int> table;
  table.singleWrite("Edvin", 2);
  table.singleWrite("Lukas", 4);
  cout << "Edvin : "<< *table.singleRead("Edvin") << endl;
  cout << "Lukas : << " << *table.singleRead("Lukas") << endl;

}
