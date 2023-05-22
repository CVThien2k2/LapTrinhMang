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
#include <ctype.h>
void trim(char *s);
void capitalize(char *s);
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

    fd_set fdread;

    int clients[64];
    char nameClients[64][50];
    int num_clients = 0;
    bool visited[64] = {false};

    char buf[256];

    while (1)
    {
        // Xóa tất cả socket trong tập fdread
        FD_ZERO(&fdread);

        // Thêm socket listener vào tập fdread
        FD_SET(listener, &fdread);
        int maxdp = listener + 1;

        // Thêm các socket client vào tập fdread
        for (int i = 0; i < num_clients; i++)
        {
            FD_SET(clients[i], &fdread);
            if (maxdp < clients[i] + 1)
                maxdp = clients[i] + 1;
        }

        // Chờ đến khi sự kiện xảy ra
        int ret = select(maxdp, &fdread, NULL, NULL, NULL);

        if (ret < 0)
        {
            perror("select() failed");
            return 1;
        }

        // Kiểm tra sự kiện có yêu cầu kết nối
        if (FD_ISSET(listener, &fdread))
        {

            int client = accept(listener, NULL, NULL);
            printf("Có kết nối mới: %d\n", client);
            char rq[256];
            int msg_len = snprintf(rq, sizeof(rq), "Xin chào, Hiện có %d client đang kết nối", num_clients + 1);
            int s = send(client, rq, strlen(rq), 0);
            clients[num_clients++] = client;
        }

        // Kiểm tra sự kiện có dữ liệu truyền đến socket client
        for (int i = 0; i < num_clients; i++)
        {

            if (FD_ISSET(clients[i], &fdread))
            {
                ret = recv(clients[i], buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    // TODO: Client đã ngắt kết nối, xóa client ra khỏi mảng
                    continue;
                }
                buf[ret] = 0;
                char *str = buf;
                char *out = str, *end = str + strlen(str);
                // Loại bỏ khoảng trắng đầu câu
                while (isspace(*str))
                {
                    str++;
                }
                // Xóa khoảng trắng cuối câu
                while (isspace(*(--end)))
                    ;
                *(++end) = '\0';
                // Dịch các từ và xóa khoảng trắng thừa giữa các từ
                char prev_char = ' ';
                while (*str)
                {
                    if (!isspace(*str) || (prev_char && !isspace(prev_char)))
                    {
                        *(out++) = *str;
                    }
                    prev_char = *str++;
                }
                printf("%s", str);
            }
        }
    }

    close(listener);

    return 0;
}
