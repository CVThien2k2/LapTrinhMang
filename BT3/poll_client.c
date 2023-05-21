#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/select.h>

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("connect() failed");
        return 1;
    }
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = client;
    fds[1].events = POLLIN;
    char buf[256];

    while (1)
    {

        int ret = poll(fds, 2, -1);

        if (fds[0].revents & POLLIN)
        { // Nếu có dữ liệu từ bàn phím
            fgets(buf, sizeof(buf), stdin);
            send(client, buf, strlen(buf), 0);
        }

        // Kiểm tra sự kiện có dữ liệu từ socket
        if (fds[1].revents & POLLIN)
        { // Nếu có dữ liệu từ socket
            {
                ret = recv(client, buf, sizeof(buf), 0);
                buf[ret] = 0;
                if (ret <= 0)
                {
                    break;
                }
                printf("%s\n", buf);
            }
        }
    }

    close(client);

    return 0;
}