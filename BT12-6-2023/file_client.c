#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

#define MAX_BUFFER_SIZE 1024

void receive_file(int socket) {
    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received;

    // Receive the server response
    while ((bytes_received = recv(socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }
}

int main() {
    int client_socket;
    struct sockaddr_in server_address;
    struct hostent *server;

    // Create the client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error in creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr, "Error: No such host\n");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(12345);
    memcpy(&server_address.sin_addr.s_addr, server->h_addr, server->h_length);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error in connecting to server");
        exit(EXIT_FAILURE);
    }

    // Send the request to the server
    char request[MAX_BUFFER_SIZE] = "LIST";
    send(client_socket, request, strlen(request), 0);

    // Receive the file list or error message from the server
    receive_file(client_socket);

    // Close the client socket
    close(client_socket);

    return 0;
}
