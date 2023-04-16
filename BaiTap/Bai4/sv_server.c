#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Error input\n");
        exit(1);
    }
    char *file_log = argv[2];

    FILE *log_file = fopen(file_log, "a");

    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serv_sock == -1)
    {
        printf("Error socket!\n");
        return 0;
    }

    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("Error Bind()!\n");
        return 0;
    }

    if (listen(serv_sock, 5) == -1)
    {
        printf("Error listen()!\n");
        return 0;
    }

    struct sockaddr_in client;
    socklen_t client_addr_size = sizeof(client);
    printf("Đang chờ client kết nối...!\n");
    int clnt_sock = accept(serv_sock, (struct sockaddr *)&client, &client_addr_size);
    if (clnt_sock == -1)
    {
        printf("Error accept!");
    }
    printf("Đã kết nối!\n");
    char buf[256];
    while (1)
    {

        int str_len = recv(clnt_sock, buf, sizeof(buf), 0);
        if (str_len <= 0 )
        {
            printf("Disconnect!\n");
            return 0;
        }

        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        char s[20];
        strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", tm);

        char msg[1000];
        sprintf(msg, "%s %s %s\n",inet_ntoa(client.sin_addr),s,buf );
        fwrite(msg,1,strlen(msg),log_file);
        printf("%s\n",msg);
    }

    fclose(log_file);
    close(clnt_sock);
    return 0;
}
