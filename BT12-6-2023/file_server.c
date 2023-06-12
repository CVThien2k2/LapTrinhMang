#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>

#define MAX_BUFFER_SIZE 1024

void send_file_list(int client_socket) {
    // Open the directory
    DIR *dir = opendir("."); // Replace "./folder" with the actual directory path

    if (dir == NULL) {
        // Directory does not exist or cannot be opened
        send(client_socket, "ERRORNoDirectory\r\n", strlen("ERRORNoDirectory\r\n"), 0);
        close(client_socket);
        return;
    }

    struct dirent *entry;
    int file_count = 0;
    char file_list[256] = "";

    // Iterate through the directory entries
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Add the file name to the file list
        strcat(file_list, entry->d_name);
        strcat(file_list, "\r\n");
        file_count++;
    }

    closedir(dir);

    // Construct the response
    char response[1024] = "";
    sprintf(response, "OK %d\r\n%s\r\n\r\n", file_count, file_list);

    // Send the response to the client
    send(client_socket, response, strlen(response), 0);
}

void send_file(int client_socket, char *filename) {
    FILE *file;
    char buffer[MAX_BUFFER_SIZE];
    size_t bytes_read;

    file = fopen(filename, "rb");
    if (file == NULL) {
        send(client_socket, "ERRORFileNotFound\r\n", strlen("ERRORFileNotFound\r\n"), 0);
        close(client_socket);
        return;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    sprintf(buffer, "OK %ld\r\n", file_size);
    send(client_socket, buffer, strlen(buffer), 0);

    // Send the file content
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }

    fclose(file);
    close(client_socket);
}

void handle_client(int client_socket) {
    char request[MAX_BUFFER_SIZE];
    ssize_t bytes_received;

    // Receive client request
    bytes_received = recv(client_socket, request, sizeof(request) - 1, 0);
    if (bytes_received < 0) {
        perror("Error in receiving request");
        close(client_socket);
        return;
    }

    // Null-terminate the received data
    request[bytes_received] = '\0';

    // Check if the request is for file list
    if (strcmp(request, "LIST") == 0) {
        send_file_list(client_socket);
    } else {
        // Extract the requested filename
        char *filename = strtok(request, " ");
        if (filename == NULL) {
           send_file_list(client_socket);
            close(client_socket);
        }

        // Check if the requested file exists
        if (access(filename, F_OK) == 0) {
            send_file_list(client_socket);
        } else {
            send(client_socket, "Gửi lại tên file\r\n", strlen("Gửi lại tên file\r\n"), 0);
            close(client_socket);
            return;
        }
    }
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_size;

    // Create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error in creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(12345);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the server socket to a specific address and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error in binding");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) < 0) {
        perror("Error in listening");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port 12345...\n");

    while (1) {
        // Accept a new client connection
        client_address_size = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_size);
        if (client_socket < 0) {
            perror("Error in accepting client connection");
            exit(EXIT_FAILURE);
        }

        // Fork a new process to handle the client connection
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            close(server_socket);
            handle_client(client_socket);
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("Error in forking");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            close(client_socket);
        }
    }

    // Close the server socket
    close(server_socket);

    return 0;
}
