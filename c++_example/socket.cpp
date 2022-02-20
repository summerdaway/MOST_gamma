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

using namespace std;

#define USER "gamma"
#define PASSWORD "bP68MDyR"

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

const int N = 512;
const int64_t M = 20220209192254;

const int BUFFER_SIZE = 1 * 1024 * 1024 * 1024; // 1GB
uint8_t *buffer = NULL;

// http头
char headers[1024];

// 获取当前时间，单位：秒
inline double get_timestamp()
{
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main()
{
    // {
    //     cpu_set_t mask;
    //     CPU_ZERO(&mask);
    //     CPU_SET(78, &mask);
    //     rt_assert_eq(pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask), 0);
    // }

    buffer = new uint8_t[BUFFER_SIZE];

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    printf("socket_fd = %d\n", socket_fd);

    // connect
    {
        struct sockaddr_storage addr;
        memset(&addr, 0, sizeof(addr));

        struct sockaddr_in *addr_v4 = (struct sockaddr_in *)&addr;
        addr_v4->sin_family = AF_INET;
        addr_v4->sin_port = htons(10001);

        rt_assert_eq(inet_pton(AF_INET, "47.95.111.217", &addr_v4->sin_addr), 1);

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

    // get first data
    int64_t pos = 0;
    {
        while (pos < N + 256)
        {
            ssize_t n = read(socket_fd, buffer + pos, 1024);
            rt_assert(n > 0);
            pos += n;
        }
        printf("get first N data success\n");
    }

    int64_t num[N + 10];
    // 使用目前收到的数据，生成最新的N个不同长度的数据
    for (int i = 1; i <= N; i++)
    {
        num[i] = 0;
        for (int j = pos - i; j < pos; j++)
        {
            num[i] = (num[i] * 10 + buffer[j] - '0') % M;
        }
    }

    int64_t pow10[N + 10];
    pow10[0] = 1;
    for (int i = 1; i <= N; i++)
    {
        pow10[i] = (pow10[i - 1] * 10) % M;
    }

    // main loop
    {
        // 非阻塞
        rt_assert_eq(fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL) | O_NONBLOCK), 0);

        while (pos + 1024 < BUFFER_SIZE)
        {
            ssize_t n = read(socket_fd, buffer + pos, 1024);

            // 用户计时
            double received_time = get_timestamp();
            double sent_times[1024];
            int sent_times_cnt = 0;

            if (n == -1)
            {
                if (errno != EAGAIN)
                {
                    printf("Read Error\n");
                    exit(-1);
                }
                continue;
            }
            if (n == 0)
            {
                printf("EOF\n");
                exit(-1);
            }

            int to_close_fds[1024];
            int to_close_fds_cnt = 0;

            for (int i = pos; i < pos + n; i++)
            {
                int64_t ch = buffer[i] - '0';
                for (int k = 1; k <= N; k++)
                {
                    // 这里会算出负数，但是并不影响
                    num[k] = (num[k] * 10 + ch - (buffer[i - k] - '0') * pow10[k]) % M;
                }
                int ans_k = -1;
                for (int k = 1; k <= N; k++)
                {
                    if (unlikely(num[k] == 0))
                    {
                        if (likely(buffer[i - k + 1] != '0'))
                        {
                            ans_k = k;
                            break;
                        }
                    }
                }
                if (ans_k >= 0)
                {
                    int k = ans_k;
                    int headers_n = sprintf(headers, "POST /submit?user=%s&passwd=%s HTTP/1.1\r\nContent-Length: %d\r\n\r\n", USER, PASSWORD, k);
                    memcpy(headers + headers_n, buffer + (i - k + 1), k);
                    sent_times[sent_times_cnt++] = get_timestamp();

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

                        rt_assert_eq(inet_pton(AF_INET, "47.95.111.217", &addr_v4->sin_addr), 1);

                        rt_assert_eq(connect(socket_fd, (struct sockaddr *)addr_v4, sizeof(*addr_v4)), 0);
                    }

                    int ret = write(socket_fd, headers, headers_n + k);
                    if (ret != headers_n + k)
                    {
                        printf("=====\n");
                        exit(-1);
                    }
                    close(socket_fd);
                }
            }

            if (sent_times_cnt > 0)
            {
                for (int i = 0; i < sent_times_cnt; i++)
                {
                    printf("%.6lf ", sent_times[i] - received_time);
                }
                printf("\n");
            }
            pos += n;
        }
    }

    return 0;
}