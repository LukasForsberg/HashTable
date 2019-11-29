#include <thread>
#include <iostream>
#include <vector>

using namespace std;

vector<int> v;

void foo(int i){
    static int a = 0;
    atomic_noexcept {
      a++;
      if(a == i){
        v[a] = a;
      } else {
        v[a] = i;
      }
    }
}

int main (){
  size_t no_threads = 10;
  v.resize(10);
  thread* threads = new thread[no_threads];

  for(int i = 0; i < no_threads; i++){
    threads[i] = thread(foo,i);
  }

  for(int i = 0; i < no_threads; i++){
    threads[i].join();
  }

  for(auto x : v){
    cout << x << endl;
  }

  return 0;
}
