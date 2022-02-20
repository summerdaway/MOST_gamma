#include <iostream>
#include <cstdint>
#include <atomic>
#include <cstring>
#include <unordered_map>

// #include "trie.h"
#include "bign.h"
#include "bign1.h"

using namespace std;

const int N = 256;

// #define LOGINFO

const int max_L = 2500;
const int L = 2000; // len of update new coming
const char* _M = "125000000000000064750000000000009507500000000000294357";
const char* _phi_M = "125000000000000064000000000000009250000000000000275600";
const char* _rev10ML = "104334427414884780086302544628657663943852728296858967";
// char *primes[] = {"500000000000000021", "500000000000000107", "500000000000000131"};

template <typename T>
class Most {
public:

    T M, phi_M;

    T S;
    int hash = 0;
    int l_passed;
    T _pow10[N+10];
    T rev_pow10_10[max_L+5][10];
    // unordered_map<int, int> hash_idx;
    int hash_idx[4194304];
    bool is_even = false;
    int sent_st, sent_ed;
    bign guess_nums[N+10], guess_pow10[N+10][10], M_base10;
    int M_len;


    void init(const char* _M, const char* _phi_M, const char* _rev10ML) {
        M = _M;
        M_base10 = _M;
        M_len = strlen(_M);
        printf("M=%s\n", M.str().c_str());

        _pow10[0] = 1;
        for (int i = 1; i < N + 10; ++i) {
            _pow10[i] = _pow10[i-1]*10;
            while (_pow10[i] >= M) _pow10[i].minus(M);
        }

        // phi_M = _M;
        // for (int i = 0; i < n_primes; ++i) {
        //     phi_M = phi_M / primes[i];
        // }
        // printf("divide=%s\n", phi_M.str().c_str());
        // for (int i = 0; i < n_primes; ++i) {
        //     phi_M = phi_M * (primes[i]-1) % M;
        // }

        phi_M = _phi_M;

        printf("phi(M)=%s\n", phi_M.str().c_str());

        T rev_pow10[max_L+5];
        // bign bign10 = 10;
        // bign rev10 = fast_mod(bign10, phi_M - 1, M);
        T rev10ML;
        rev10ML = _rev10ML;
        rev_pow10[max_L] = rev10ML;
        for (int i = max_L-1; i >= 0; --i) {
            rev_pow10[i] = rev_pow10[i+1] * 10;
            while (rev_pow10[i] >= M) rev_pow10[i].minus(M);
            // printf("i=%d %s\n", i, rev_pow10[i].str().c_str());
        }
        // for (int i = 1; i < L + 2; ++i) {
        //     rev_pow10[i] = rev_pow10[i-1] * rev10 % M;
        // }
        printf("rev10=%s\n", rev_pow10[1].str().c_str());
        assert(rev_pow10[0] == 1);
        for (int i = 0; i < L+2; ++i) {
            for (int j = 0; j < 10; ++j) {
                rev_pow10_10[i][j] = rev_pow10[i] * j;
                while (rev_pow10_10[i][j] >= M) rev_pow10_10[i][j].minus(M);
            }
        }

        // guess init
        for (int k = 0; k < 10; ++k) {  
            guess_pow10[0][k] = k;
        }
        for (int i = 1; i <= N; ++i) {
            for (int j = 0; j < 10; ++j) {
                guess_pow10[i][j] = guess_pow10[i-1][j] * 10;
                while (guess_pow10[i][j] >= M_base10) guess_pow10[i][j].minus(M_base10);
            }
        }
        for (int i = 0; i < 10; ++i) {
            printf("10^10*%d=%s\n", i, guess_pow10[10][i].str().c_str()); 
        }
    }

    inline string guess_func(int ed, char* buffer) {
        // printf("ed=%d\n", ed);
        // buffer[ed] = '\0';
        // printf("%s\n", buffer);
        sent_st = sent_ed = 0;
        bign guess_nums[N+1];
        guess_nums[N] = 0;
        for (int i = N; i > 0; --i) {
            guess_nums[N] = guess_nums[N] * 10 + (buffer[ed-i]-'0');
            while (guess_nums[N] >= M_base10) guess_nums[N].minus(M_base10);
        }
        // printf("%s\n", guess_nums[N].str().c_str());
        for (int i = N - 1; i > 0; --i) {
            // printf("i=%d %d %s\n", i, buffer[ed-i] - '0', guess_pow10[i][buffer[ed-i]-'0'].str().c_str());
            if (guess_nums[i+1] >= guess_pow10[i][buffer[ed-i-1]-'0']) {
                guess_nums[i] = guess_nums[i+1] - guess_pow10[i][buffer[ed-i-1]-'0'];
            } else {
                // printf("i=%d\n", i);
                // printf("%s\n%s\n", guess_nums[i+1].str().c_str(), guess_pow10[i][buffer[ed-i-1]-'0'].str().c_str());
                guess_nums[i] = guess_nums[i+1] + M_base10;
                guess_nums[i].minus(guess_pow10[i][buffer[ed-i-1]-'0']);
            }
        }
        // for (int i = N; i >= 0; --i) {
        //     printf("%s\n", guess_nums[i].str().c_str());
        // }
        for (int i = N-1; i > 0; --i) {
            if (buffer[ed-i] == '0') continue;
            int l = min(N - i, M_len - 10);
            for (int j = 1; j <= l; ++j) {
                guess_nums[i] = guess_nums[i] * 10;
                while (guess_nums[i] >= M_base10) guess_nums[i].minus(M_base10);
                bign x = M_base10 - guess_nums[i];
                if (x.get_len() <= j) {
                    sent_st = ed - i;
                    sent_ed = ed + j;
                    string ret = "";
                    if (x.get_len() < j) {
                        for (int k = j-x.get_len(); k > 0; --k) {
                            ret += "0";
                        }
                    }
                    return ret + x.str();
                }
            }
        }
        return "";
    }

    inline void update_stats(int ed, const char* buffer) {
        // hash_idx.clear();
        memset(hash_idx, -1, sizeof(hash_idx));
        S = 0;
        hash = 0;
        for (int i = N; i > 0; --i) {
            if (buffer[ed-i] == '0') continue;
            // trie.insert(S, ed-i);
            hash_idx[hash & 4194303] = ed-i;
            S.add(rev_pow10_10[N-i][buffer[ed-i]-'0']);
            // S = S + rev_pow10_10[N-i][buffer[ed-i]-'0'];
            // while (S >= M) S.minus(M);
            if (S >= M) S.minus(M);
            hash = S.hash() & 4194303;
        }
    }

    inline int find() {
        return hash_idx[hash];
        // auto itr = hash_idx.find(hash);
        // if (unlikely(itr != hash_idx.end())) {
        //     return itr->second;
        // }
        // return -1;
    }

    inline void update_S(int idx, int l, int x) {
        hash_idx[hash] = idx;
        S.add(rev_pow10_10[l][x]);
        if (S >= M) S.minus(M);
        hash = S.hash() & 4194303;
    }

};