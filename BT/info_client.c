#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

struct Maytinh
{
    char tenmaytinh[20];
    int CountDisk;
    struct Disk
    {
        char namedisk[10];
        int kich_thuoc;
    } disk[10];
};

int main()
{

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Error!\n");
        return 0;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(9000);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("Error!\n");
        return 0;
    }
    printf("Đã kết nối!!\n");
    while (1)
    {
        struct Maytinh maytinh;
        printf("\n\n**Nhập thông tin máy tính, nếu muốn thoát ấn tổ hợp Ctrl + C**\n\n");

        printf("+Nhập tên máy tính: ");
        scanf("%s", maytinh.tenmaytinh);

        printf("Nhập số ổ đĩa: ");
        scanf(" %d", &maytinh.CountDisk);

        for (int i = 0; i < maytinh.CountDisk; i++)
        {
            printf("Nhập ký tự ổ đĩa thứ %d: ", i + 1);
            scanf("%s", maytinh.disk[i].namedisk);
            printf("Nhập kích thước ổ đĩa thứ %d: ", i + 1);
            scanf("%d", &maytinh.disk[i].kich_thuoc);
        }

        char message[256];
        sprintf(message, "%s/%d", maytinh.tenmaytinh, maytinh.CountDisk);

        int pos = strlen(message);
        for (int i = 0; i < maytinh.CountDisk; i++)
        {
            int result = snprintf(message + pos, sizeof(message) - pos, "/%s/%d", maytinh.disk[i].namedisk, maytinh.disk[i].kich_thuoc);
            if (result >= sizeof(message) - pos || result < 0)
            {
                // Xử lý lỗi
                break;
            }
            pos += result;
        }
        printf("%s", message);

        write(sock, message, strlen(message));
    }

    close(sock);
    return 0;
}