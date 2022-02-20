#define num_M 4
const char* M[num_M] = {
    "2022021721441", 
    "104648257118348370704723119", 
    "125000000000000140750000000000052207500000000006359661", 
    "10885732038215355481752285039386319187390558900925053798518152998488201"
};
const char* phi_M[num_M] = {
    "2017266922320", 
    "104648257118348370704723118", 
    "125000000000000140000000000000051646000000000006255808", 
    "5654925734137847003507680539940945032410679948532495479749689869344520"
};
const char* rev10_ML[num_M] = {
    "1066286657516", 
    "69479802049515323581601510", 
    "17245912421121535800014577319986943281736054046017986", 
    "8137105143825576462125417902668770373777246613990144794468348801785665"
};

#include <iostream>
#include <cstdint>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <atomic>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>


#include "rt_assert.h"
#include "most.h"
#include "most_ll.h"

#define ACTUAL_SEND
// #define LOGINFO

#define ADDR "172.1.1.119"
#define USER "gamma"
#define PASSWORD "bP68MDyR"


#define n_socket_input 8
#define n_socket_post 16
int socket_input[n_socket_input], socket_post[n_socket_post];
// http头
char headers[n_socket_post][1024];
int common_header_length = 0;
int socket_post_used;
int sent_times_cnt;
double sent_times[1024];
// #define RECONNECT 3

const int BUFFER_SIZE = 128 * 1024 * 1024; // 1GB
char buffer[n_socket_input][BUFFER_SIZE];
int pos[n_socket_input];

// const char* keep_alive = "POST /submit?user=a";


int l_passed;
int last_post;

Most_LL processor1;
Most<bign_m2> processor2;
Most<bign_m3> processor3;
Most<bign_m4> processor4;


// 获取当前时间，单位：秒
inline double get_timestamp()
{
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}


inline int connect_post() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    rt_assert(socket_fd >= 0);

    // no delay
    {
        int flag = 1;
        int result = setsockopt(socket_fd,     /* socket affected */
                                IPPROTO_TCP,   /* set option at TCP level */
                                TCP_NODELAY,   /* name of option */
                                (char *)&flag, /* the cast is historical cruft */
                                sizeof(int));  /* length of option value */
        rt_assert_eq(result, 0);
    }


    // connect
    {
        struct sockaddr_storage addr;
        memset(&addr, 0, sizeof(addr));

        struct sockaddr_in *addr_v4 = (struct sockaddr_in *)&addr;
        addr_v4->sin_family = AF_INET;
        addr_v4->sin_port = htons(10002);

        rt_assert_eq(inet_pton(AF_INET, ADDR, &addr_v4->sin_addr), 1);

        rt_assert_eq(connect(socket_fd, (struct sockaddr *)addr_v4, sizeof(*addr_v4)), 0);
    }
    return socket_fd;
}


inline int connect_input() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    printf("socket_fd = %d\n", socket_fd);

    // connect
    {
        struct sockaddr_storage addr;
        memset(&addr, 0, sizeof(addr));

        struct sockaddr_in *addr_v4 = (struct sockaddr_in *)&addr;
        addr_v4->sin_family = AF_INET;
        addr_v4->sin_port = htons(10001);

        rt_assert_eq(inet_pton(AF_INET, ADDR, &addr_v4->sin_addr), 1);

        rt_assert_eq(connect(socket_fd, (struct sockaddr *)addr_v4, sizeof(*addr_v4)), 0);

        printf("connect success\n");
    }

    // skip http header
    {
        char tmp[1024];
        while (true)
        {
            memset(tmp, 0, sizeof(tmp));
            ssize_t n = read(socket_fd, tmp, sizeof(tmp));
            rt_assert(n > 0);
            if (strstr(tmp, "\r\n"))
                break;
        }
        printf("skip http header success\n");
    }

    return socket_fd;
}


int socket_post_id = 0;
inline void post(int st, int ed, const char *buffer) {
    if (ed == last_post) return;
    last_post = ed;
    // socket_post = connect_post();

    int k = ed - st;
    int headers_n = sprintf(headers[socket_post_id] + common_header_length, "%d\r\n\r\n", k);
    headers_n += common_header_length;
    memcpy(headers[socket_post_id] + headers_n, buffer + st, k);
#ifdef LOGINFO
    sent_times[sent_times_cnt] = get_timestamp();
    ++sent_times_cnt;
#endif

    headers[socket_post_id][headers_n+k] = '\0';

#ifdef LOGINFO
    printf("send %s socket_post=%d time=%.6lf\n", headers[socket_post_id] + headers_n, socket_post[socket_post_id], sent_times[sent_times_cnt - 1]);
    printf("send finished time=%.6lf\n", get_timestamp());
#endif

#ifdef ACTUAL_SEND
    int ret = -1;
    // check_and_reconnect();
    // assert(check_connection(socket_post));
    ret = write(socket_post[socket_post_id], headers[socket_post_id], headers_n + k);
    if (ret != headers_n + k) {
        printf("%d %d %d %d=====\n", ret, headers_n + k, headers_n, k);
        cout << errno << endl;
        exit(-1);
    }
#endif

    socket_post_id = (socket_post_id + 1) & 15;
    // close(socket_post);
}

inline void bind_cpu(int cpu)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);
    assert(pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) == 0);
}

inline int get_input_min_pos() {
    int ret = pos[0];
    for (int i = 1; i < n_socket_input; ++i) {
        ret = min(ret, pos[i]);
    }
    return ret;
}

int main()
{
    // {
    //     cpu_set_t mask;
    //     CPU_ZERO(&mask);
    //     CPU_SET(78, &mask);
    //     rt_assert_eq(pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask), 0);
    // }

    // bind_cpu(0);

    processor1.init(M[0], phi_M[0], rev10_ML[0]);
    processor2.init(M[1], phi_M[1], rev10_ML[1]);
    processor3.init(M[2], phi_M[2], rev10_ML[2]);
    processor4.init(M[3], phi_M[3], rev10_ML[3]);

    // init connections
    // socket_input = connect_input();
    for (int i = 0; i < n_socket_input; ++i) {
        socket_input[i] = connect_input();
    }
    for (int i = 0; i < n_socket_post; ++i) {
        socket_post[i] = connect_post();
        common_header_length = sprintf(headers[i], "POST /submit?user=%s&passwd=%s HTTP/1.1\r\nContent-Length: ", USER, PASSWORD);
    }

    // for (int i = 0; i < n_socket_input; ++i)
    //     buffer[i] = new char[BUFFER_SIZE];
    // init processors

    while (true) {
        usleep(1000000);
        // get first data
        for (int i = 0; i < n_socket_input; i ++)
            read(socket_input[i], buffer[i], 10240);
        for (int i = 0; i < n_socket_input; i ++)
            read(socket_input[i], buffer[i], 10240);

        for (int i = 0; i < n_socket_input; ++i)
            memset(buffer[i], 0, sizeof(BUFFER_SIZE));
        memset(pos, 0, sizeof(pos));

        while (get_input_min_pos() < N + 256)
        {
            for (int i = 0; i < n_socket_input; i ++) {
                ssize_t n = read(socket_input[i], buffer[i] + pos[i], 10240);
                rt_assert(n > 0);
                pos[i] += n;
                buffer[i][pos[i]] = '\0';
                printf("%d %d %s\n", i, pos[i], buffer[i]);
            }
        }
        printf("get first N data success\n");

        bool flag = true;
        int check_len = get_input_min_pos();
        printf("check_len = %d\n", check_len);
        for (int j = 0; j < check_len; j ++) {
            bool is_same = true;
            for (int i = 1; i < n_socket_input; i ++)
                is_same &= (buffer[i][j] == buffer[0][j]);
            flag &= is_same;
        }
        if (flag)
            break;
    }


    int pos_k = 0;
    for (int i = 0; i < n_socket_input; i ++) {
        if (pos[i] > pos[pos_k])
            pos_k = i;
        printf("%d %d %s\n", i, pos[i], buffer[i]);
    }
    
    processor1.update_stats(pos[pos_k], buffer[pos_k]);
    processor2.update_stats(pos[pos_k], buffer[pos_k]);
    printf("finish init\n");

    // main loop
    {
        // 非阻塞
        for (int i = 0; i < n_socket_input; i ++)
            rt_assert_eq(fcntl(socket_input[i], F_SETFL, fcntl(socket_input[i], F_GETFL) | O_NONBLOCK), 0);
#ifdef LOGINFO
        double last_log_timestamp = get_timestamp();
        double total_times = 0, max_send_time = 0;
        int total_send_cnt = 0, total_numbers = 0;
#endif

        while (true) {
            for (int k = 0; k < n_socket_input; k ++) {
                ssize_t n = read(socket_input[k], buffer[k] + pos[k], 1024);

                if (n <= 0) continue;

                if (pos[k] + n <= pos[pos_k]) {
                    pos[k] += n;
                    continue;
                }

                n = pos[k] + n - pos[pos_k];
                pos[k] = pos[pos_k];
                pos_k = k;

#ifdef LOGINFO
                printf("k = %d, n = %d\n", k, n);


                // 用户计时
                double received_time = get_timestamp();
                sent_times_cnt = 0;
#endif

                // socket_post_used = 0;

                {
                    int prv_idx = -1, ch, idx;
                    for (int i = pos[k]; i < pos[k] + n; i++) {
                        ch = buffer[k][i] - '0';
                        if (ch != 0) {
                            processor1.update_S(i, i-pos[k]+N, ch);
                            processor2.update_S(i, i-pos[k]+N, ch);
                            processor3.update_S(i, i-pos[k]+N, ch);
                            processor4.update_S(i, i-pos[k]+N, ch);
                        } else if (unlikely(i-prv_idx+1 <= N)) {
                            post(prv_idx, i + 1, buffer[k]);
                            // ++socket_post_used;
                            continue;
                        }
                        prv_idx = -1;

                        // if (unlikely(socket_post_used >= n_socket_post)) continue;
                        idx = processor2.find();
                        if (unlikely(i-idx < N)) {
                            post(idx, i + 1, buffer[k]);
                            prv_idx = idx;
                            // ++socket_post_used;
#ifdef LOGINFO
                            printf("pos=%ld\n", i-pos[k]);
#endif
                            continue;
                        }


                        idx = processor3.find();
                        if (unlikely(i-idx < N)) {
                            post(idx, i + 1, buffer[k]);
                            prv_idx = idx;
                            // ++socket_post_used;
#ifdef LOGINFO
                            printf("pos=%ld\n", i-pos[k]);
#endif
                            continue;
                        }

                        idx = processor4.find();
                        if (unlikely(i-idx < N)) {
                            post(idx, i + 1, buffer[k]);
                            prv_idx = idx;
                            // ++socket_post_used;
#ifdef LOGINFO
                            printf("pos=%ld\n", i-pos[k]);
#endif
                            continue;
                        }
                        
                        if (ch == 0) {
                            idx = processor1.find();
                            if (unlikely(i-idx < N)) {
                                post(idx, i + 1, buffer[k]);
                                prv_idx = idx;
                                // ++socket_post_used;
#ifdef LOGINFO
                                printf("pos=%ld\n", i-pos[k]);
#endif
                            }
                        }
                    }
                }

                pos[k] += n;
                string s = processor2.guess_func(pos[k], buffer[k]);
                if (processor2.sent_st > 0) {
                    const char *guess_ans = s.c_str();
                    sprintf(buffer[k] + pos[k], guess_ans, sizeof(guess_ans));
                    post(processor2.sent_st, processor2.sent_ed, buffer[k]);
                    // printf("%d %d %d\n", processor2.sent_st, processor2.sent_ed, processor2.sent_ed-processor2.sent_st);
                    // printf("%s %s\n", buffer[k] + processor2.sent_st, s.c_str());
                }
                s = processor3.guess_func(pos[k], buffer[k]);
                if (processor3.sent_st > 0) {
                    const char *guess_ans = s.c_str();
                    sprintf(buffer[k] + pos[k], guess_ans, sizeof(guess_ans));
                    post(processor3.sent_st, processor3.sent_ed, buffer[k]);
                    // printf("%d %d %d\n", processor3.sent_st, processor3.sent_ed, processor3.sent_ed-processor3.sent_st);
                    // printf("%s %s\n", buffer[k] + processor3.sent_st, s.c_str());
                }

                s = processor4.guess_func(pos[k], buffer[k]);
                if (processor4.sent_st > 0) {
                    const char *guess_ans = s.c_str();
                    sprintf(buffer[k] + pos[k], guess_ans, sizeof(guess_ans));
                    post(processor4.sent_st, processor4.sent_ed, buffer[k]);
                    // printf("%d %d %d\n", processor4.sent_st, processor4.sent_ed, processor4.sent_ed-processor4.sent_st);
                    // printf("%s %s\n", buffer[k] + processor4.sent_st, s.c_str());
                }


                
                processor1.update_stats(pos[k], buffer[k]);
                processor2.update_stats(pos[k], buffer[k]);
                processor3.update_stats(pos[k], buffer[k]);
                processor4.update_stats(pos[k], buffer[k]);

#ifdef LOGINFO
                if (sent_times_cnt > 0)
                {
                    for (int i = 0; i < sent_times_cnt; i++) {
                        max_send_time = max(max_send_time, sent_times[i] - received_time);
                        total_times += sent_times[i] - received_time;
                    }
                    total_send_cnt += sent_times_cnt;
                }
                printf("process %lu numbers.\n", n);
                total_numbers += n;
                if (true || received_time > last_log_timestamp + 10) {
                    if (total_send_cnt > 0)
                        printf("process %d numbers, send %d answers, mean = %.3lf us, max = %.3lf us\n", total_numbers, total_send_cnt, total_times / total_send_cnt * 1e6, max_send_time * 1e6);
                    last_log_timestamp = received_time;
                    total_times = 0;
                    total_send_cnt = 0;
                    total_numbers = 0;
                    max_send_time = 0;
                }
#endif
            }
        }
    }

    return 0;
}

