#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

struct Student
{
    char MSSV[20];
    char HoTen[50];
    int NgaySinh;
    int ThangSinh;
    int NamSinh;
    float DiemTBC;
} ;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Error!\n");
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Error!\n");
        return 0;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("Error!\n");
        return 0;
    }
    printf("Đã kết nối!!\n");
    while (1)
    {
        struct Student student;
        printf("\n\n**Nhập thông tin sinh viên, nếu muốn thoát ấn tổ hợp Ctrl + C**\n\n");

        printf("Nhap MSSV: ");
        scanf("%s", student.MSSV);

        printf("Nhap ho ten: ");
        scanf(" %[^\n]", student.HoTen);

        printf("Nhap ngay sinh: ");
        scanf("%d", &student.NgaySinh);

        printf("Nhap thang sinh: ");
        scanf("%d", &student.ThangSinh);

        printf("Nhap nam sinh: ");
        scanf("%d", &student.NamSinh);

        printf("Nhap diem trung binh: ");
        scanf("%f", &student.DiemTBC);

        char message[256];
        sprintf(message, "%s %s %d-%02d-%02d %.2f", student.MSSV, student.HoTen, student.NamSinh, student.ThangSinh, student.NgaySinh, student.DiemTBC);

        write(sock, message, strlen(message));
    }

    close(sock);
    return 0;
}