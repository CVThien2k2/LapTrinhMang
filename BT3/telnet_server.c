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
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
bool Check(char *username, char *password);
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

    char username[64][50], Password[64][50];
    bool visited[64] = {false};
    char buf[256];
    char *request = "Vui lòng nhập tài khoản và mật khẩu của bạn(đúng dạng username password): ";

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
            fds[nfds].fd = client;
            fds[nfds].events = POLLIN;
            nfds++;
            int s = send(client, request, strlen(request), 0);
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
                    if (strchr(buf, ' ') == NULL)
                    {
                        send(fds[i].fd, request, strlen(request), 0);
                    }
                    else
                    {
                        char pass[20];
                        char name[20];
                        sscanf(buf, "%s %s", name, pass);
                        if (Check(name, pass))
                        {
                            strcpy(username[i], name);
                            strcpy(Password[i], pass);

                            printf("%s đã kết nối tới Server chat.\n", name);
                            visited[i] = true;
                            send(fds[i].fd, "Kết nốt thành công, vui lòng nhập lệnh: ", strlen("Kết nốt thành công, vui lòng nhập lệnh: "), 0);
                        }
                        else
                        {
                            send(fds[i].fd, "Thông tin tài khoản hoặc mật khẩu không chính xác! Vui lòng nhập lại: ", strlen("Thông tin tài khoản hoặc mật khẩu không chính xác! Vui lòng nhập lại: "), 0);
                        }
                    }
                }
                else
                {
                    buf[ret] = 0;
                    printf("Lệnh client yêu cầu là: %s\n", buf);
                    char full_msg[500];
                    int msg_len = snprintf(full_msg, sizeof(full_msg), "%s>text.txt",buf);
                    int r = system(full_msg);
                    send(fds[i].fd, "Vui lòng nhập lệnh: ", strlen("Vui lòng nhập lệnh: "), 0);
                }
            }
        }
    }

    close(listener);

    return 0;
}
bool Check(char *username, char *password)
{
    char buffer[256];
    char name[50], pass[50];
    FILE *f = fopen("database.txt", "r");
    if (f == NULL)
    {
        perror("Error!");
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, sizeof(buffer), f) != NULL)
    {
        sscanf(buffer, "%s %s", name, pass);

        if (strcmp(username, name) == 0 && strcmp(password, pass) == 0)
        {
            return true;
        }
    }
    return false;
}