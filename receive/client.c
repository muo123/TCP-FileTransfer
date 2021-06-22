#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define BUFFER_SIZE 1024

#define ERR_EXIT(m)         \
    do                      \
    {                       \
        perror(m);          \
        exit(EXIT_FAILURE); \
    } while (0)

int main()
{
    int client_fd;
    char stdin_buf[BUFFER_SIZE], ip_name_str[INET_ADDRSTRLEN];
    uint16_t port_num;
    char recv_buf[BUFFER_SIZE], send_buf[BUFFER_SIZE], filename[BUFFER_SIZE];
    int sendbytes, recvbytes, ret;
    char clr;
    struct hostent *host;
    struct sockaddr_in server_addr, connect_addr;
    socklen_t addr_len;
    FILE *fp = NULL;

    printf("Input server's hostname/ipv4: "); /* www.baidu.com or an ipv4 address */
    scanf("%s", stdin_buf);
    while ((clr = getchar()) != '\n' && clr != EOF)
        ; /* clear the stdin buffer */
    printf("Input server's port number: ");
    scanf("%hu", &port_num);
    while ((clr = getchar()) != '\n' && clr != EOF)
        ;

    if ((host = gethostbyname(stdin_buf)) == NULL)
    {
        printf("invalid name or ip-address\n");
        exit(EXIT_FAILURE);
    }

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        ERR_EXIT("socket()");
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_num);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(server_addr.sin_zero), 8);

    addr_len = sizeof(struct sockaddr);
    ret = connect(client_fd, (struct sockaddr *)&server_addr, addr_len); /* connect to server */
    if (ret == -1)
    {
        close(client_fd);
        ERR_EXIT("connect()");
    }
    else
    {
        printf("connected\n");
    }

    ret = getsockname(client_fd, (struct sockaddr *)&connect_addr, &addr_len);
    if (ret == -1)
    {
        close(client_fd);
        ERR_EXIT("getsockname()");
    }
    port_num = ntohs(connect_addr.sin_port);
    strcpy(ip_name_str, inet_ntoa(connect_addr.sin_addr));
    printf("Local port: %hu, IP addr: %s\n", port_num, ip_name_str);

    while (1)
    {
        memset(send_buf, 0, BUFFER_SIZE);
        printf("\ninput the file name: ");
        fgets(send_buf, BUFFER_SIZE, stdin);

        sendbytes = send(client_fd, send_buf, BUFFER_SIZE-1, 0);
        if (sendbytes <= 0)
        {
            printf("sendbytes = %d. Connection terminated ...\n", sendbytes);
            break;
        }

        memset(filename, 0, BUFFER_SIZE);
        strcpy(filename, send_buf);
        int length = strlen(filename);
        filename[length-1] = 0;

        memset(recv_buf, 0, BUFFER_SIZE);

        while (recvbytes = recv(client_fd, recv_buf, BUFFER_SIZE-1, 0))
        {
            if (recvbytes <= 0)
            {
                printf("recvbytes = %d. Connection terminated ...\n", recvbytes);
                break;
            }

            if (strncmp(recv_buf, "#0", 2) == 0)
            {
                printf("server: no such file\n");
                break;
            }
            if (strncmp(recv_buf, "#1", 2) == 0)
            {
                fp = fopen(filename, "w");
                if (fp == NULL)
                {
                    ERR_EXIT("fopen()");
                }
                memset(recv_buf, 0, BUFFER_SIZE);

                printf("downloading...\n\n");
                struct stat msg;
                recvbytes = recv(client_fd, recv_buf, BUFFER_SIZE-1, 0);
                if (recvbytes <= 0)
                {
                    printf("recvbytes = %d. Connection terminated ...\n", recvbytes);
                    break;
                }
                memcpy(&msg, &recv_buf, sizeof(recv_buf));
                printf(">>>file name: %s", filename);
                printf(">>>the file size = %ld bytes\n", msg.st_size);
                printf(">>>the file uid: %d\n", msg.st_uid);
                printf(">>>the file type: %d\n", msg.st_mode);
                printf(">>>last modify time: %s\n", ctime(&msg.st_mtime));

                memset(recv_buf, 0, BUFFER_SIZE);
            }

            if (strncmp(recv_buf, "#2", 2) == 0)
            {
                fclose(fp);
                printf("download sucessfully ^-^...\n");
                break;
            }
            fwrite(recv_buf, sizeof(char), strlen(recv_buf), fp);
            memset(recv_buf, 0, BUFFER_SIZE);
        }
    }

    close(client_fd);
    return EXIT_SUCCESS;
}