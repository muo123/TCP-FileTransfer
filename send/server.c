#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <time.h>

#define BUFFER_SIZE 1024

#define ERR_EXIT(m)         \
    do                      \
    {                       \
        perror(m);          \
        exit(EXIT_FAILURE); \
    } while (0)

int getipv4addr(char *ip_addr)
{
    struct ifaddrs *ifaddrsptr = NULL;
    struct ifaddrs *ifa = NULL;
    void *tmpptr = NULL;
    int ret;

    ret = getifaddrs(&ifaddrsptr);
    if (ret == -1)
        ERR_EXIT("getifaddrs()");

    for (ifa = ifaddrsptr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
        {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET)
        { /* IP4 */
            tmpptr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addr_buf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpptr, addr_buf, INET_ADDRSTRLEN);
            printf("%s IPv4 address %s\n", ifa->ifa_name, addr_buf);
            if (strcmp(ifa->ifa_name, "lo") != 0)
                strcpy(ip_addr, addr_buf); /* return the ipv4 address */
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        { /* IP6 */
            tmpptr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addr_buf[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpptr, addr_buf, INET6_ADDRSTRLEN);
            printf("%s IPv6 address %s\n", ifa->ifa_name, addr_buf);
        }
    }

    if (ifaddrsptr != NULL)
    {
        freeifaddrs(ifaddrsptr);
    }

    return EXIT_SUCCESS;
}

int main()
{
    int server_fd, client_fd;
    char stdin_buf[BUFFER_SIZE], ip4_addr[INET_ADDRSTRLEN];
    struct sockaddr_in server_addr, client_addr;
    uint16_t port_num;
    socklen_t addr_len;
    int ret;
    char recv_buf[BUFFER_SIZE], send_buf[BUFFER_SIZE], filename[BUFFER_SIZE];
    int recvbytes, sendbytes;
    FILE *fp;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        ERR_EXIT("socket()");
    }
    printf("server_fd = %d\n", server_fd);

    getipv4addr(ip4_addr);

    printf("input server port number: ");
    memset(stdin_buf, 0, BUFFER_SIZE);
    fgets(stdin_buf, 6, stdin);
    stdin_buf[5] = 0;
    port_num = atoi(stdin_buf);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_num);
    server_addr.sin_addr.s_addr = inet_addr(ip4_addr);
    bzero(&(server_addr.sin_zero), 8);

    addr_len = sizeof(struct sockaddr);
    ret = bind(server_fd, (struct sockaddr *)&server_addr, addr_len);
    if (ret == -1)
    {
        close(server_fd);
        ERR_EXIT("bind()");
    }
    printf("Bind success!\n");

    ret = listen(server_fd, 1);
    if (ret == -1)
    {
        close(server_fd);
        ERR_EXIT("listen()");
    }
    printf("Listening ...\n");

    addr_len = sizeof(struct sockaddr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd == -1)
    {
        ERR_EXIT("accept()");
    }

    port_num = ntohs(client_addr.sin_port);
    strcpy(ip4_addr, inet_ntoa(client_addr.sin_addr));
    printf("Client IP_addr = %s, port = %hu\n", ip4_addr, port_num);

    while (1)
    { /* receiving and sending cycle */
        memset(recv_buf, 0, BUFFER_SIZE);
        recvbytes = recv(client_fd, recv_buf, BUFFER_SIZE-1, 0); /* non-blocking socket recv */
        if (recvbytes <= 0)
        {
            printf("recvbytes = %d. Connection terminated ...\n", recvbytes);
            break;
        }

        memset(filename, 0, BUFFER_SIZE);
        strcpy(filename, recv_buf);
        int length = strlen(recv_buf);
        filename[length-1] = 0;

        memset(send_buf, 0, BUFFER_SIZE);

        fp = fopen(filename, "r");
        if (fp == NULL)
        {
            printf("File not found: %s\n", filename);

            strcpy(send_buf, "#0\n");
            sendbytes = send(client_fd, send_buf, BUFFER_SIZE-1, 0); /* blocking socket send */
            if (sendbytes <= 0)
            {
                break;
            }

            printf("waiting for file request...\n\n");
        }
        else
        {
            strcpy(send_buf, "#1\n");
            sendbytes = send(client_fd, send_buf, BUFFER_SIZE-1, 0); /* blocking socket send */
            if (sendbytes <= 0)
            {
                break;
            }
            printf("transfering...\n");

            struct stat msg;
            stat(filename, &msg);
            printf(">>>file name: %s\n", filename);
            printf(">>>the file size = %ld bytes\n", msg.st_size);
            printf(">>>the file uid: %d\n", msg.st_uid);
            printf(">>>the file type: %d\n", msg.st_mode);
            printf(">>>last modify time: %s\n", ctime(&msg.st_mtime));

            memset(send_buf, 0, BUFFER_SIZE);
            memcpy(send_buf, &msg, sizeof(struct stat));
            sendbytes = send(client_fd, send_buf, BUFFER_SIZE-1, 0); /* blocking socket send */
            if (sendbytes <= 0)
            {
                break;
            }
            memset(send_buf, 0, BUFFER_SIZE);

            while (fread(send_buf, sizeof(char), BUFFER_SIZE-1, fp))
            {
                sendbytes = send(client_fd, send_buf, BUFFER_SIZE-1, 0); /* blocking socket send */
                if (sendbytes <= 0)
                {
                    printf("file send failed\n");
                    break;
                }
                memset(send_buf, 0, BUFFER_SIZE);
            }

            strcpy(send_buf, "#2\n");
            sendbytes = send(client_fd, send_buf, BUFFER_SIZE-1, 0); /* blocking socket send */
            if (sendbytes <= 0)
            {
                break;
            }

            fclose(fp);
            printf("transfer finished ^-^...\n\n");
        }
    }

    close(client_fd);
    close(server_fd);
    exit(EXIT_SUCCESS);
}