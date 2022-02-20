#include <iostream>
#include <cstdint>
#include <atomic>
#include <cstring>
#include <unordered_map>

using namespace std;

class Most_LL {
public:

    int64_t M, phi_M;

    int64_t S;
    int l_passed;
    int64_t _pow10[N+10];
    int64_t rev_pow10_10[max_L+5][10];
    // unordered_map<int64_t, int> hash_idx;
    int hash_idx[4194304];
    bool is_even = false;

    int64_t M_x[11];

    inline int64_t char_to_ll(const char* x) {
        int64_t ret = 0;
        int n = strlen(x);
        for (int i = 0; i < n; ++i) {
            ret = ret * 10;
            ret += x[i] - '0';
        }
        return ret;
    }

    void init(const char* _M, const char* _phi_M, const char* _rev10ML) {
        // M = _M;
        M = char_to_ll(_M);
        printf("M=%ld\n", M);

        _pow10[0] = 1;
        for (int i = 1; i < N + 10; ++i) {
            _pow10[i] = _pow10[i-1]*10 % M;
        }

        // phi_M = _M;
        // for (int i = 0; i < n_primes; ++i) {
        //     phi_M = phi_M / primes[i];
        // }
        // printf("divide=%s\n", phi_M.str().c_str());
        // for (int i = 0; i < n_primes; ++i) {
        //     phi_M = phi_M * (primes[i]-1) % M;
        // }

        phi_M = char_to_ll(_phi_M);
        printf("phi(M)=%ld\n", phi_M);

        int64_t rev_pow10[max_L+5];
        // bign bign10 = 10;
        // bign rev10 = fast_mod(bign10, phi_M - 1, M);
        int64_t rev10ML;
        rev10ML = char_to_ll(_rev10ML);
        rev_pow10[max_L] = rev10ML;
        for (int i = max_L-1; i >= 0; --i) {
            rev_pow10[i] = rev_pow10[i+1] * 10 % M;
        }
        // for (int i = 1; i < L + 2; ++i) {
        //     rev_pow10[i] = rev_pow10[i-1] * rev10 % M;
        // }
        printf("rev10=%ld\n", rev_pow10[1]);
        assert(rev_pow10[0] == 1);
        for (int i = 0; i < L+2; ++i) {
            for (int j = 0; j < 10; ++j) {
                rev_pow10_10[i][j] = rev_pow10[i] * j % M;
            }
        }
    }

    void update_stats(int ed, const char* buffer) {
        // hash_idx.clear();
        memset(hash_idx, -1, sizeof(hash_idx));
        S = 0;
        for (int i = N; i > 0; --i) {
            if (buffer[ed-i] == '0') continue;
            // trie.insert(S, ed-i);
            hash_idx[S & 4194303] = ed-i;
            S += rev_pow10_10[N-i][buffer[ed-i]-'0'];
            // S = S + rev_pow10_10[N-i][buffer[ed-i]-'0'];
            // while (S >= M) S.minus(M);
            if (S >= M) S -= M;
        }
    }

    inline int find() {
        return hash_idx[S & 4194303];
    }

    inline void update_S(int idx, int l, int x) {
        hash_idx[S & 4194303] = idx;
        S += rev_pow10_10[l][x];
        if (S >= M) S -= M;
    }

};