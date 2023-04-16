#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Error!");
        return 1;
    }
    int port = atoi(argv[1]);
    char *hello_file = argv[2];
    char *save_file = argv[3];

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

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

    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(addr);

    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    printf("Client IP: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    FILE *f1 = fopen(hello_file, "rb");
    char hello[256];
    int ret = fread(hello, 1, sizeof(hello), f1);
    if (ret <= 0)
    {
        printf("Error!");
        return 0;
    }
    send(client, hello, strlen(hello), 0);

    char buf[256];
    FILE *f2 = fopen(save_file, "wb");

    while (1)
    {
        int res = recv(client, buf, sizeof(buf), 0);
        if (res <= 0)
        {
            printf("Ket noi bi dong.\n");
            break;
        }
        if (res < sizeof(buf))
            buf[res] = 0;
        fwrite(buf, 1, res, f2);

        printf("Tin nhan da duoc them vao file %s: ", save_file);
        printf("%s\n", buf);
    }
    fclose(f1);
    fclose(f2);
    close(client);
    close(listener);
}