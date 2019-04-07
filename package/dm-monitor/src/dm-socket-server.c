#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <fcntl.h>

#include <jansson.h>

#include "mcompat_drv.h"
#include "mcompat_lib.h"
#include "dm-socket-server.h"

static void PrintBytes(const char *buffer, int bufLen);
static int Parse(const char *buffer, int bufLen);

void *thread_socket_server(void *argv)
{
    int rst = 0;
    /*
     * 监听后，一直处于accept阻塞状态,
     * 直到有客户端连接,
     * 当客户端如数quit后，断开与客户端的连接 */

    /* 调用socket函数返回的文件描述符 */
    int serverSocket = 0;

    /* 声明两个套接字sockaddr_in结构体变量，分别表示客户端和服务器 */
    struct sockaddr_in server_addr;
    struct sockaddr_in clientAddr;
    int addr_len = sizeof(clientAddr);
    int client = 0;
    char *sendBuf = NULL;
    char recvBuff[DM_RECV_BUF_SIZE] = {0};
    int recvBytes = 0;

    /*
     * socket函数，失败返回-1
     * int socket(int domain, int type, int protocol);
     * 第一个参数表示使用的地址类型，一般都是ipv4，AF_INET
     * 第二个参数表示套接字类型：tcp：面向连接的稳定数据传输SOCK_STREAM
     * 第三个参数设置为0 */
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket < 0)
    {
        perror("socket");
        return NULL;
    }

    bzero(&server_addr, sizeof(server_addr));

    /* 初始化服务器端的套接字，并用htons和htonl将端口和地址转成网络字节序
     * ip可是是本服务器的ip，也可以用宏INADDR_ANY代替，代表0.0.0.0，表明所有地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DM_MONITOR_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 对于bind，accept之类的函数，里面套接字参数都是需要强制转换成(struct sockaddr *)
     * bind三个参数：服务器端的套接字的文件描述符.  */
    if(bind(serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        return NULL;
    }

    /* 设置服务器上的socket为监听状态 */
    if(listen(serverSocket, DM_LISTEN_NUM) < 0)
    {
        perror("listen");
        return NULL;
    }

    while(1)
    {
        /* printf("Listening all IP on port: %d\n", DM_MONITOR_PORT); */

        /*
         * 调用accept函数后，会进入阻塞状态
         * accept返回一个套接字的文件描述符，这样服务器端便有两个套接字的文件描述符，
         * serverSocket和client。
         * serverSocket仍然继续在监听状态，client则负责接收和发送数据
         * clientAddr是一个传出参数，accept返回时，传出客户端的地址和端口号
         * addr_len是一个传入-传出参数，传入的是调用者提供的缓冲区的clientAddr的长度，以避免缓冲区溢出。
         * 传出的是客户端地址结构体的实际长度。
         * 出错返回-1 */
        client = accept(serverSocket, (struct sockaddr*)&clientAddr, (socklen_t*)&addr_len);
        if(client < 0)
        {
            perror("accept");
            continue;
        }

        /* 接收:
         * inet_ntoa ip地址转换函数，将网络字节序IP转换为点分十进制IP
         * 表达式：char *inet_ntoa (struct in_addr); */
        /* printf("recv from:%s:%d.\n", inet_ntoa(clientAddr.sin_addr), htons(clientAddr.sin_port)); */
        bzero(recvBuff, DM_RECV_BUF_SIZE);
        recvBytes = recv(client, recvBuff, DM_RECV_BUF_SIZE, 0);
        if(recvBytes < 0)
        {
            perror("recv");
            continue;
        }
        if(recvBytes >= DM_RECV_BUF_SIZE)
        {
            fprintf(stderr, "DM_RECV_BUF_SIZE is too small.");
            return NULL;
        }

        /*
        printf("recv data %d bytes is:\n", recvBytes);
        PrintBytes(recvBuff, recvBytes);
        */

        rst = Parse(recvBuff, recvBytes);
        if(0 == rst)
        {
            sendBuf = "{\"success\": \"true\"}";
        }
        else
        {
            sendBuf = "{\"success\": \"false\"}";
        }

        send(client, sendBuf, strlen(sendBuf) + 1, 0);
        /* PrintBytes(sendBuf, strlen(sendBuf) + 1); */
    }
    return NULL;
}

static int Parse(const char *buffer, int bufLen)
{
    /*
     * API:
     * {red_light: 'on'}
     * {red_light: 'off'}
     * {red_light: 'twinkle'}
     *
     * */
    json_t *root = NULL;
    json_t *red_light_val = NULL;
    const char *red_light_val_s = NULL;

    /* step1: parse json */
    json_error_t error;
    root = json_loads(buffer, 0, &error);
    if(!root)
    {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return -1;
    }

    /* step2: get value */
    red_light_val = json_object_get(root, "red_light");
    if(NULL == red_light_val)
    {
        fprintf(stderr, "error: red_light not impl.\n");
        json_decref(root);
        return -1;
    }
    if(!json_is_string(red_light_val))
    {
        fprintf(stderr, "error: red_light_val is not a string.\n");
        json_decref(root);
        return -1;
    }

    red_light_val_s = json_string_value(red_light_val);
    /* printf("red_light_val is %s\n", red_light_val_s); */

    /* step3: run function */
    /* step3.1: red light function */
    if(0 == strncmp(red_light_val_s, "on", strlen(red_light_val_s)))
    {
        mcompat_set_red_led(0);
        printf("red_light_val is on.\n");
    }
    else if(0 == strncmp(red_light_val_s, "off", strlen(red_light_val_s)))
    {
        mcompat_set_red_led(1);
        printf("red_light_val is off.\n");
    }
    else if(0 == strncmp(red_light_val_s, "twinkle", strlen(red_light_val_s)))
    {
        printf("red_light_val is twinkle(not impl).\n");
        json_decref(root);
        return -1;
    }
    else
    {
        fprintf(stderr, "error: red_light_val is not a valid val:%s.\n", red_light_val_s);
        json_decref(root);
        return -1;
    }

    /* step3: release */
    json_decref(root);

    return 0;
}

static void PrintBytes(const char *buffer, int bufLen)
{
    int i = 0;

    for(i = 0; i< bufLen; i++)
    {
        printf("%02x,", buffer[i]);
    }
    printf("\n");

    for(i = 0; i< bufLen; i++)
    {
        if( 0 != buffer[i])
        {
            printf("%c", buffer[i]);
        }
        else
        {
            break;
        }
    }
    printf("\n");
}

