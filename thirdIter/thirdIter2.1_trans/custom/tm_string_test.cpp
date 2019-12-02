#include <cassert>
#include <iostream>
#include "tm_string.h"

void test1(){
  tm_string s1;
  assert(s1.length()== 0);

  tm_string s2 = "Lukas";
  s1 = s2;
  assert(s2 == "Lukas" && s2.length() == 5);
  assert(s1 == "Lukas");
  assert(s1 == s2);
  assert(s1[0] == 'L');

  s1[0] = 'K';
  assert(s1[0] == 'K');
  assert(s2[0] == 'L');
  assert( s1 != s2);

  cout << "test1 : OK" << endl;;
}


int main(){
  test1();
}
