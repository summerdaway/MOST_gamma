#ifndef __BIGN_H__
#define __BIGN_H__

#include <stdio.h>
#include <stdlib.h>
#include <iostream>  
#include <string>  
#include <cstring>  
#include <cstdio>  
using namespace std;  

#define BASE_10 10

// const int maxn = 200;
// const int prime1 = 49157;
// // const int prime2 = 98317;
// const int HASH_LEN = 18;
// // const int HASH_MOD2 = 1000000007;
// const int HASH_MOD2 = 50331653;

struct bign{
    int d[200], len;  
  
    void clean() { while(len > 1 && !d[len-1]) len--; }  
  
    bign()          { memset(d, 0, sizeof(d)); len = 1; }  
    bign(int num)   { *this = num; }   
    bign(char* num) {
        *this = num;
        // printf("this=%s\n", this->str().c_str());
    }  
    bign operator = (const char* num){ 
        bign s(0);
        for (int i = 0; i < strlen(num); i ++) {
            s = s * 10 + (num[i] - '0');
            // printf("%s\n", s.str().c_str());
        }
        memcpy(d, s.d, sizeof(s.d));
        len = s.len;
        return s;
        
        // memset(d, 0, sizeof(d)); len = strlen(num);  
        // for(int i = 0; i < len; i++) d[i] = num[len-1-i] - '0';  
        // clean();  
        // return *this;  
    }  
    bign operator = (int num){
        // char s[20]; sprintf(s, "%d", num);  
        // *this = s;  
        // return *this;  
        memset(d, 0, sizeof(d));
        len = 0;
        while (num > 0) {
            d[len ++] = num % BASE_10;
            num /= BASE_10;
        }
        if (len == 0) len = 1;
        clean();
        return *this;
    }  
  
    bign operator + (const bign& b){  
        bign c = *this; int i;  
        for (i = 0; i < b.len; i++){  
            c.d[i] += b.d[i];  
            if (c.d[i] >= BASE_10) c.d[i]-=BASE_10, c.d[i+1]++;  
        }  
        while (c.d[i] >= BASE_10) c.d[i++]-=BASE_10, c.d[i]++;  
        c.len = max(len, b.len);  
        if (c.d[i] && c.len <= i) c.len = i+1;  
        return c;  
    }  

    inline void add(const bign &b) {
        int i;
        for (i = 0; i < b.len; ++i){  
            d[i] += b.d[i];  
            if (d[i] >= BASE_10) d[i]-=BASE_10, ++d[i+1];  
        }  
        while (d[i] >= BASE_10) {
            d[i] -= BASE_10;
            ++i;
            ++d[i];
        }
        len = max(len, b.len);
        if (d[i] > 0 && len <= i) len = i+1;
    }

    bign operator - (const bign& b){  
        bign c = *this; int i;  
        for (i = 0; i < b.len; i++){  
            c.d[i] -= b.d[i];  
            if (c.d[i] < 0) c.d[i]+=BASE_10, c.d[i+1]--;  
        }  
        while (c.d[i] < 0) c.d[i++]+=BASE_10, c.d[i]--;  
        c.clean();  
        return c;  
    }  

    inline void minus(const bign &b) {
        int i;  
        for (i = 0; i < b.len; ++i){  
            d[i] -= b.d[i];  
            if (d[i] < 0) d[i]+=BASE_10, --d[i+1];  
        }  
        while (d[i] < 0) d[i++]+=BASE_10, --d[i];  
        clean();  
    }

    bign operator * (const bign& b)const{  
        int i, j; bign c; c.len = len + b.len;   
        for(j = 0; j < b.len; j++) for(i = 0; i < len; i++)   
            c.d[i+j] += d[i] * b.d[j];  
        for(i = 0; i < c.len-1; i++)  
            c.d[i+1] += c.d[i]/BASE_10, c.d[i] %= BASE_10;  
        c.clean();  
        return c;  
    }  
    // bign operator / (const bign& b){  
    //     int i, j;  
    //     bign c = *this, a = 0;  
    //     for (i = len - 1; i >= 0; i--)  
    //     {  
    //         a = a*BASE + d[i];  
    //         for (j = 0; j < BASE; j++) if (a < b*(j+1)) break;  
    //         c.d[i] = j;  
    //         a = a - b*j;  
    //     }  
    //     c.clean();  
    //     return c;  
    // }  

    // bign operator % (const bign& b){  
    //     int i, j;  
    //     bign a = 0;  
    //     for (i = len - 1; i >= 0; i--)  
    //     {  
    //         a = a*BASE + d[i];  
    //         for (j = 0; j < BASE; j++) if (a < b*(j+1)) break;  
    //         a = a - b*j;  
    //     }  
    //     return a;  
    // }  
    bign* operator += (const bign& b){  
        // *this = *this + b;  
        return this;  
    }  
  
    bool operator <(const bign& b) const{  
        if(len != b.len) return len < b.len;  
        for(int i = len-1; i >= 0; --i)  
            if(d[i] != b.d[i]) return d[i] < b.d[i];  
        return false;  
    }  
    bool operator >(const bign& b) const{return b < *this;}  
    bool operator<=(const bign& b) const{return !(b < *this);}  
    inline bool operator>=(const bign& b) const{
        if(len != b.len) return len > b.len;  
        for(int i = len-1; i >= 0; --i)  
            if(d[i] != b.d[i]) return d[i] > b.d[i];  
        return true;
    }  
    bool operator!=(const bign& b) const{return b < *this || *this < b;}  
    bool operator==(const bign& b) const{return !(b < *this) && !(b > *this);}  

    string str() const{  
        char s[200]={};  
        for(int i = 0; i < len; i++) s[len-1-i] = d[i]+'0';  
        return s;  
    }  

    // bign left_shift(int x) {
    //     for (int i = len; i > 0; --i) {
    //         d[i] = d[i-1];
    //     }
    //     d[0] = x;
    //     len += 1;
    //     clean();
    //     return *this;
    // }

    // bool is_zero() {
    //     return len == 1 && d[0] == 0;
    // }

    // inline int hash() {
    //     int h1 = 0; //, h2 = 0;
    //     for (int i = 0; i < len; ++i) {
    //         // h = (h * prime + d[i]) % HASH_MOD2;
    //         h1 = h1 * prime1 + d[i];
    //         // h2 = h2 * prime2 + d[i];
    //     }
    //     // return h1^h2;
    //     return h1;
    // }

    inline int get_len() {
        return len;
        int ret = str().size();
        // printf("len=%d\n", ret);
        return ret;
        // char s[10];
        // sprintf(s, "%d", d[len-1]);
        // return (len-1)*8 + strlen(s);
    }
};  


#endif

