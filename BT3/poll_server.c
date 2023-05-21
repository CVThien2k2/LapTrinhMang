#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <stdbool.h>
#include <poll.h>

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    struct pollfd fds[64];
    int nfds = 1;
    fds[0].fd = listener;
    fds[0].events = POLLIN;

    char nameClients[64][50];
    bool visited[64] = {false};

    char buf[256];

    while (1)
    {

        int ret = poll(fds, nfds, -1);
        if (ret < 0)
        {
            perror("select() failed");
            return 1;
        }
        if (fds[0].revents & POLLIN)
        { // Sự kiện có kết nối mới
            int client = accept(listener, NULL, NULL);
            printf("New client connected %d\n", client);
            fds[nfds].fd = client;
            fds[nfds].events = POLLIN;
            nfds++;
            printf("Có kết nối mới: %d\n", client);
            int s = send(client, "Vui lòng nhập tên của bạn (đúng định dạng client_id:name):", strlen("Vui lòng nhập tên của bạn (đúng định dạng client_id:name):"), 0);
        }

        // Kiểm tra sự kiện có dữ liệu truyền đến socket client
        for (int i = 0; i < nfds; i++)
        {

            if (fds[i].revents & (POLLIN | POLLERR))
            {
                ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    // TODO: Client đã ngắt kết nối, xóa client ra khỏi mảng
                    continue;
                }
                buf[ret] = 0;
                if (visited[i] == false)
                {
                    if (strchr(buf, ':') == NULL)
                    {
                        send(fds[i].fd, "Vui lòng nhập tên của bạn (đúng định dạng client_id:name):", strlen("Vui lòng nhập tên của bạn (đúng định dạng client_id:name):"), 0);
                    }
                    else
                    {
                        char id[20];
                        char name[20];
                        char *token = strtok(buf, ":");
                        strcpy(id, token);
                        token = strtok(NULL, ":");
                        token[strlen(token) - 1] = 0;
                        strcpy(name, token);
                        if (strcmp(id, "client_id") != 0 || strlen(name) < 3)
                        {
                            send(fds[i].fd, "Vui lòng nhập tên của bạn (đúng định dạng client_id:name):", strlen("Vui lòng nhập tên của bạn (đúng định dạng client_id:name):"), 0);
                        }
                        else
                        {
                            strcpy(nameClients[i], name);
                            printf("%s đã kết nối tới Server chat.\n", name);
                            visited[i] = true;
                        }
                    }
                }
                else
                {
                    buf[ret] = 0;
                    // printf(" %s : %s\n", nameClients[i], buf);
                    char full_msg[500];
                    int msg_len = snprintf(full_msg, sizeof(full_msg), "%s: %s", nameClients[i], buf);
                    full_msg[strlen(full_msg) - 1] = 0;
                    for (int j = 1; j < nfds; j++)
                    {
                        if (j != i && visited[j] == true)
                        {
                            send(fds[j].fd, full_msg, strlen(full_msg), 0);
                        }
                    }
                }
            }
        }
    }

    close(listener);

    return 0;
}
