#ifndef __BIGN_H1__
#define __BIGN_H1__

#include <stdio.h>
#include <stdlib.h>
#include <iostream>  
#include <string>  
#include <cstring>  
#include <cstdio>  
using namespace std;  



#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#define BASE 576460752303423488LL
#define BASE_BIT 59L
#define BASE_AND 576460752303423487LL


const int maxn = 4;
const int prime1 = 49157;
// const int prime2 = 98317;
// const int HASH_LEN = 18;
// const int HASH_MOD2 = 1000000007;
// const int HASH_MOD2 = 50331653;



class bign_m2{
public:

    int64_t d[2];

    bign_m2()          { memset(d, 0, sizeof(d)); }  

    bign_m2(int num)   { *this = num; }   
    bign_m2(char* num) {
        *this = num;
        // printf("this=%s\n", this->str().c_str());
    }  

    inline void add(const bign_m2 &b) {
        d[0] += b.d[0];
        d[1] += b.d[1] + (d[0] >> BASE_BIT);
        d[0] &= BASE_AND;
    }

    inline void minus(const bign_m2 &b) {
        d[0] -= b.d[0];
        d[1] -= b.d[1] - (d[0] >> BASE_BIT);
        d[0] &= BASE_AND;
    }

    inline int hash() {
        return d[0] ^ d[1];
    }

    
    bign_m2 operator = (const char* num){ 
        bign_m2 s(0);
        for (int i = 0; i < strlen(num); i ++) {
            s = s * 10 + (num[i] - '0');
        }
        memcpy(d, s.d, sizeof(s.d));
        return s;
        
        // memset(d, 0, sizeof(d)); len = strlen(num);  
        // for(int i = 0; i < len; i++) d[i] = num[len-1-i] - '0';  
        // clean();  
        // return *this;  
    }  
    
    bign_m2 operator = (int num){
        // char s[20]; sprintf(s, "%d", num);  
        // *this = s;  
        // return *this;  
        memset(d, 0, sizeof(d));
        d[0] = num;
        return *this;
    }  
  
    bign_m2 operator + (const bign_m2& b){  
        bign_m2 c = *this; 
        c.add(b);
        return c;
    }  
    
    bign_m2 operator * (int b)const{
        // b <= 10
        bign_m2 c = *this;
        c.d[0] *= b;
        c.d[1] *= b;
        c.d[1] += c.d[0] / BASE;
        c.d[0] %= BASE;
        return c;  
    }  



    inline bool operator>=(const bign_m2& b) const{
        if (unlikely(d[1] != b.d[1])) return d[1] > b.d[1];
        return d[0] >= b.d[0];
    }  

    bool operator==(const bign_m2& b) const{return (b>=*this) && (*this>=b);}  


    string str() const{  
        char s[200]={};  
        sprintf(s, "%018ld %018ld", d[1], d[0]);
        // sprintf(s + 18, "%018ld", d[0]); 
        return s;  
    }  
};  




class bign_m3{
public:

    int64_t d[3];
    bign_m3()          { memset(d, 0, sizeof(d)); }  

    bign_m3(int num)   { *this = num; }   
    bign_m3(char* num) {
        *this = num;
        // printf("this=%s\n", this->str().c_str());
    }  

    inline void add(const bign_m3 &b) {
        d[0] += b.d[0];
        d[1] += b.d[1] + (d[0] >> BASE_BIT);
        d[2] += b.d[2] + (d[1] >> BASE_BIT);
        d[0] &= BASE_AND;
        d[1] &= BASE_AND;
    }

    inline void minus(const bign_m3 &b) {
        d[0] -= b.d[0];
        d[1] -= b.d[1] - (d[0] >> BASE_BIT);
        d[2] -= b.d[2] - (d[1] >> BASE_BIT);
        d[0] &= BASE_AND;
        d[1] &= BASE_AND;
    }

    inline int hash() {
        return d[0] ^ d[1] ^ d[2];
    }

    
    bign_m3 operator = (const char* num){ 
        bign_m3 s(0);
        for (int i = 0; i < strlen(num); i ++) {
            s = s * 10 + (num[i] - '0');
        }
        memcpy(d, s.d, sizeof(s.d));
        return s;
        
        // memset(d, 0, sizeof(d)); len = strlen(num);  
        // for(int i = 0; i < len; i++) d[i] = num[len-1-i] - '0';  
        // clean();  
        // return *this;  
    }  

    bign_m3 operator = (int num){
        // char s[20]; sprintf(s, "%d", num);  
        // *this = s;  
        // return *this;  
        memset(d, 0, sizeof(d));
        d[0] = num;
        return *this;
    }  
  
    bign_m3 operator + (const bign_m3& b){  
        bign_m3 c = *this; 
        c.add(b);
        return c;
    }  

    bign_m3 operator * (int b)const{
        // b <= 10
        bign_m3 c = *this;
        c.d[0] *= b;
        c.d[1] *= b;
        c.d[2] *= b;
        c.d[1] += c.d[0] / BASE;
        c.d[2] += c.d[1] / BASE;
        c.d[0] %= BASE;
        c.d[1] %= BASE;
        return c;  
    }  



    inline bool operator>=(const bign_m3& b) const{
        if (unlikely(d[2] != b.d[2])) return d[2] > b.d[2];
        if (unlikely(d[1] != b.d[1])) return d[1] > b.d[1];
        return d[0] >= b.d[0];
    }  

    bool operator==(const bign_m3& b) const{return (b>=*this) && (*this>=b);}  


    string str() const{  
        char s[200]={};  
        sprintf(s, "%018ld %018ld %018ld", d[2], d[1], d[0]);
        // sprintf(s + 18, "%018ld", d[0]); 
        return s;  
    }  
};  



class bign_m4{
public:

    int64_t d[4];
    bign_m4()          { memset(d, 0, sizeof(d)); }  

    bign_m4(int num)   { *this = num; }   
    bign_m4(char* num) {
        *this = num;
        // printf("this=%s\n", this->str().c_str());
    }  

    inline void add(const bign_m4 &b) {
        d[0] += b.d[0];
        d[1] += b.d[1] + (d[0] >> BASE_BIT);
        d[2] += b.d[2] + (d[1] >> BASE_BIT);
        d[3] += b.d[3] + (d[2] >> BASE_BIT);
        d[0] &= BASE_AND;
        d[1] &= BASE_AND;
        d[2] &= BASE_AND;
    }

    inline void minus(const bign_m4 &b) {
        d[0] -= b.d[0];
        d[1] -= b.d[1] - (d[0] >> BASE_BIT);
        d[2] -= b.d[2] - (d[1] >> BASE_BIT);
        d[3] -= b.d[3] - (d[2] >> BASE_BIT);
        d[0] &= BASE_AND;
        d[1] &= BASE_AND;
        d[2] &= BASE_AND;
    }

    inline int hash() {
        return d[0] ^ d[1] ^ d[2] ^ d[3];
    }

    
    bign_m4 operator = (const char* num){ 
        bign_m4 s(0);
        for (int i = 0; i < strlen(num); i ++) {
            s = s * 10 + (num[i] - '0');
        }
        memcpy(d, s.d, sizeof(s.d));
        return s;
        
        // memset(d, 0, sizeof(d)); len = strlen(num);  
        // for(int i = 0; i < len; i++) d[i] = num[len-1-i] - '0';  
        // clean();  
        // return *this;  
    }  


    bign_m4 operator = (int num){
        // char s[20]; sprintf(s, "%d", num);  
        // *this = s;  
        // return *this;  
        memset(d, 0, sizeof(d));
        d[0] = num;
        return *this;
    }  
  
    bign_m4 operator + (const bign_m4& b){  
        bign_m4 c = *this; 
        c.add(b);
        return c;
    }  


    bign_m4 operator * (int b)const{
        // b <= 10
        bign_m4 c = *this;
        c.d[0] *= b;
        c.d[1] *= b;
        c.d[2] *= b;
        c.d[3] *= b;
        c.d[1] += c.d[0] / BASE;
        c.d[2] += c.d[1] / BASE;
        c.d[3] += c.d[2] / BASE;
        c.d[0] %= BASE;
        c.d[1] %= BASE;
        c.d[2] %= BASE;
        return c;  
    }  



    inline bool operator>=(const bign_m4& b) const{
        if (unlikely(d[3] != b.d[3])) return d[3] > b.d[3];
        if (unlikely(d[2] != b.d[2])) return d[2] > b.d[2];
        if (unlikely(d[1] != b.d[1])) return d[1] > b.d[1];
        return d[0] >= b.d[0];
    }  

    bool operator==(const bign_m4& b) const{return (b>=*this) && (*this>=b);}  


    string str() const{  
        char s[200]={};  
        sprintf(s, "%018ld %018ld %018ld %018ld", d[3], d[2], d[1], d[0]);
        // sprintf(s + 18, "%018ld", d[0]); 
        return s;  
    }  
};  




#endif

