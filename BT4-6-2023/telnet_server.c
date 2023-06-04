#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>

bool Check(char *username, char *password);

void signalHandler(int signo)
{
    int pid = wait(NULL);
    // printf("Child %d terminated.\n", pid);
}

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

    signal(SIGCHLD, signalHandler);
    bool visited = false;
    char *request = "Vui lòng nhập tài khoản và mật khẩu của bạn(đúng dạng username password): ";

    while (1)
    {
        printf("Waiting for new client...\n");
        int client = accept(listener, NULL, NULL);
        send(client, request, strlen(request), 0);
        if (fork() == 0)
        {
            // Tien trinh con
            close(listener);

            // Xu ly ket noi tu client

            char buf[256];
            while (1)
            {

                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    break;
                buf[ret-1] = 0;
                if (visited == false)
                {
                    if (strchr(buf, ' ') == NULL)
                    {
                        send(client, request, strlen(request), 0);
                    }
                    else
                    {
                        char pass[20];
                        char name[20];
                        sscanf(buf, "%s %s", name, pass);
                        if (Check(name, pass))
                        {

                            printf("%s đã kết nối tới Server chat.\n", name);
                            visited = true;
                            send(client, "Kết nốt thành công, vui lòng nhập lệnh: ", strlen("Kết nốt thành công, vui lòng nhập lệnh: "), 0);
                        }
                        else
                        {
                            send(client, "Thông tin tài khoản hoặc mật khẩu không chính xác! Vui lòng nhập lại: ", strlen("Thông tin tài khoản hoặc mật khẩu không chính xác! Vui lòng nhập lại: "), 0);
                        }
                    }
                }
                else
                {
                    buf[ret] = 0;
                    char command[256];
                    char *p = buf;
                    printf("Lệnh client yêu cầu là: %s\n", p);
                    snprintf(command, sizeof(command), "%s > out.txt", p);
                    system(command);
                     send(client, "Vui lòng nhập lệnh: ", strlen("Vui lòng nhập lệnh: "), 0);
                }
            }

            close(client);
            exit(0);
        }

        // Tien trinh cha
        close(client);
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