#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 2048

// Hàm gửi lệnh và nhận phản hồi từ server
int sendCommand(int socket, const char* command, char* response) {
    send(socket, command, strlen(command), 0);
    memset(response, 0, BUFFER_SIZE);
    recv(socket, response, BUFFER_SIZE, 0);
    printf("%s", response);
    return atoi(response);
}

// Hàm thiết lập kết nối đến server
int connectToServer(const char* serverAddress, const char* serverPort) {
    struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(serverAddress, serverPort, &hints, &servinfo);

    int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);

    freeaddrinfo(servinfo);
    return sockfd;
}

// Hàm tải file lên server
void uploadFile(int socket1, const char* filePath) {
    FILE* file = fopen(filePath, "rb");
    if (file == NULL) {
        printf("Không thể mở file.\n");
        return;
    }

    char buffer[BUFFER_SIZE];

    // Gửi lệnh PASV hoặc EPSV để thiết lập kênh dữ liệu
    char pasvCommand[] = "PASV\r\n";
    char epsvCommand[] = "EPSV\r\n";
    char response[BUFFER_SIZE];

    int dataSocket = -1;
    if (sendCommand(socket, epsvCommand, response) == 229) {
        // Lấy thông tin cổng từ phản hồi của lệnh EPSV
        char* start = strchr(response, '(');
        char* end = strchr(start, ')');
        *end = '\0';

        char* portStr = strchr(start, '|');
        int port = atoi(++portStr);

        // Thiết lập kết nối mới đến kênh dữ liệu
        struct sockaddr_in dataAddr;
        memset(&dataAddr, 0, sizeof dataAddr);
        dataAddr.sin_family = AF_INET;
        dataAddr.sin_addr.s_addr = inet_addr("192.168.3.104");
        dataAddr.sin_port = htons(port);
        printf("1");
        dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        connect(dataSocket, (struct sockaddr*)&dataAddr, sizeof dataAddr);
    }
    else if (sendCommand(socket, pasvCommand, response) == 227) {
        // Lấy thông tin IP và cổng từ phản hồi của lệnh PASV
        int ip[4], port[2];
        sscanf(response, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1]);

        // Thiết lập kết nối mới đến kênh dữ liệu
        struct sockaddr_in dataAddr;
        memset(&dataAddr, 0, sizeof dataAddr);
        dataAddr.sin_family = AF_INET;
        dataAddr.sin_addr.s_addr = inet_addr("192.168.3.104");
        dataAddr.sin_port = htons(port[0] * 256 + port[1]);

        dataSocket = socket(AF_INET, SOCK_STREAM, 0);
        connect(dataSocket, (struct sockaddr*)&dataAddr, sizeof dataAddr);
    }

    if (dataSocket == -1) {
        printf("Không thể thiết lập kênh dữ liệu.\n");
        fclose(file);
        return;
    }

    // Gửi lệnh STOR để bắt đầu quá trình tải file lên
    char storCommand[100];
    sprintf(storCommand, "STOR %s\r\n", filePath);
    sendCommand(socket, storCommand, response);

    // Đọc từng phần của file và gửi lên kênh dữ liệu
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(dataSocket, buffer, bytesRead, 0);
    }

    fclose(file);
    close(dataSocket);

    // Nhận phản hồi từ server về quá trình tải file
    memset(response, 0, BUFFER_SIZE);
    recv(socket, response, BUFFER_SIZE, 0);
    printf("%s", response);
}

int main() {
    const char* serverAddress = "192.168.3.104";
    const char* serverPort = "21";

    int sockfd = connectToServer(serverAddress, serverPort);
    char response[BUFFER_SIZE];

    // Nhận xâu chào từ server
    recv(sockfd, response, BUFFER_SIZE, 0);
    printf("%s", response);

    char username[100], password[100];

    // Nhập thông tin đăng nhập từ người dùng
    printf("Nhap username: ");
    scanf("%s", username);
    printf("Nhap password: ");
    scanf("%s", password);

    // Gửi lệnh USER
    char userCommand[100];
    sprintf(userCommand, "USER %s\r\n", username);
    sendCommand(sockfd, userCommand, response);

    // Gửi lệnh PASS
    char passCommand[100];
    sprintf(passCommand, "PASS %s\r\n", password);
    sendCommand(sockfd, passCommand, response);

    // Kiểm tra phản hồi sau khi đăng nhập
    int loginCode = atoi(response);
    if (loginCode != 230) {
        printf("Dang nhap that bai.\n");
        close(sockfd);
        return 1;
    }

    // Tải file lên server
    char filePath[100];
    printf("Nhap duong dan file: ");
    scanf("%s", filePath);
    uploadFile(sockfd, filePath);

    // Gửi lệnh QUIT để kết thúc phiên làm việc với server
    sendCommand(sockfd, "QUIT\r\n", response);

    close(sockfd);
    return 0;
}
