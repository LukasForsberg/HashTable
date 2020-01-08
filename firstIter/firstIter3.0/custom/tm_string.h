#ifndef TM_STRING_H
#define TM_STRING_H

#include <iostream>
#include <cassert>
#include <functional>
#include <thread>

using namespace std;

class tm_string {
        int size;
        char * buffer;
public:
        tm_string() {
          buffer = nullptr;
          size = 0;
        }

        tm_string(const tm_string& s){
            size = s.size;
            buffer = new char[size];
            for(int i = 0; i < size; i++){
                buffer[i] = s.buffer[i];
            }
        }

        tm_string(const char *p ){
            int i = 0;
            const char * t = p;
            while(*p++) {
                i++;
            }
            buffer = new char[i];
            int j = 0;
            for(j= 0;*t;t++,j++){
                buffer[j] = *t;
            }
            size = j;
        }

        ~tm_string(){
            delete[] buffer;
        }

        int length() const{
            if(buffer == nullptr){
                return 0;
            }
            else{
                return size;
            }
        }

        char& operator[] (unsigned int x) transaction_safe {
            return buffer[x];
        }

        void operator =(const tm_string& s) transaction_safe{
          size = s.size;
          buffer = new char[size];
          for(int i = 0; i < size; i++){
              buffer[i] = s.buffer[i];
          }
        }

        	// other methods
        friend bool operator==(const tm_string &s , const tm_string &t) transaction_safe{
            if(s.size != t.size) {
                return false;
            }
            else{
                for(int i = 0; i < s.size; i++){
                    if(s.buffer[i] != t.buffer[i]){
                        return false;
                    }
                }
            }
            return true;
        }

        // other methods
      friend bool operator!=(const tm_string &s , const tm_string &t) transaction_safe{
        return !(s == t);
      }

        friend ostream & operator<<(ostream &os, const tm_string &s){
            for(int i = 0; i < s.size; i++){
                os << s.buffer[i];
            }
            return os;
        }
};

#define tm_string_A 54059 /* a prime */
#define tm_string_B 76963 /* another prime */
#define tm_string_C 86969 /* yet another prime */
#define tm_string_FIRSTH 37 /* also prime */

namespace std
{
    template <>
    struct hash<tm_string>
    {
      size_t operator()(tm_string& s) const
      {
         unsigned h = tm_string_FIRSTH;
         for(int i = 0; i < s.length(); i++){
           h = (h * tm_string_A) ^ (s[i] * tm_string_B);
         }
         return h;
      }
    };
}
#endif
