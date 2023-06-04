#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8000
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
   

    while (1) {
        int ret = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (ret <= 0) {
            break;
        }
        
        buffer[ret] = '\0';

        if (strncmp(buffer, "GET_TIME", 8) == 0) {
            char* format = strtok(buffer + 9, " \t\n");

            if (format != NULL) {
                char formatted_time[BUFFER_SIZE];
                time_t current_time = time(NULL);
                struct tm* time_info = localtime(&current_time);

                if (strcmp(format, "[dd/mm/yyyy]") == 0) {
                    strftime(formatted_time, BUFFER_SIZE, "%d/%m/%Y", time_info);
                } else if (strcmp(format, "[dd/mm/yy]") == 0) {
                    strftime(formatted_time, BUFFER_SIZE, "%d/%m/%y", time_info);
                } else if (strcmp(format, "[mm/dd/yyyy]") == 0) {
                    strftime(formatted_time, BUFFER_SIZE, "%m/%d/%Y", time_info);
                } else if (strcmp(format, "[mm/dd/yy]") == 0) {
                    strftime(formatted_time, BUFFER_SIZE, "%m/%d/%y", time_info);
                } else {
                    strcpy(formatted_time, "Invalid format");
                }

                send(client_socket, formatted_time, strlen(formatted_time), 0);
            } else {
                send(client_socket, "Invalid command", 15, 0);
            }
        } else {
            send(client_socket, "Invalid command", 15, 0);
        }
    }

    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_length;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Failed to listen on socket");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        client_address_length = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_length);

        if (client_socket < 0) {
            perror("Failed to accept client connection");
            exit(EXIT_FAILURE);
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Connected to client: %s:%d\n", client_ip, ntohs(client_address.sin_port));

        pid_t pid = fork();
        if (pid < 0) {
            perror("Failed to create child process");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            // Child process
            close(server_socket);
            handle_client(client_socket);
            exit(EXIT_SUCCESS);
        } else {
            // Parent process
            close(client_socket);
        }
    }

    close(server_socket);

    return 0;
}
