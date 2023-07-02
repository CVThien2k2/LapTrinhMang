#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <dirent.h>

void *client_thread(void *);

void signal_handler(int signo)
{
    wait(NULL);
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

    signal(SIGPIPE, signal_handler);

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }
        printf("New client connected: %d\n", client);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    close(listener);

    return 0;
}

void send_response(int client, const char *response)
{
    send(client, response, strlen(response), 0);
}

void send_file(int client, FILE *file, const char *content_type)
{
    char response_header[2048];
    sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", content_type);
    send_response(client, response_header);

    char buf[2048];
    size_t bytesRead;
    while ((bytesRead = fread(buf, 1, sizeof(buf), file)) > 0)
    {
        send(client, buf, bytesRead, 0);
    }
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    char buf[2048];

    int ret = recv(client, buf, sizeof(buf) - 1, 0);
    if (ret <= 0)
        return NULL;

    buf[ret] = 0;
    printf("Received from %d: %s\n", client, buf);

    char method[16];
    char path[256];
    sscanf(buf, "%s %s", method, path);

    if (strcmp(path, "/") == 0)
    {
        DIR *dir = opendir(".");
        if (dir != NULL)
        {
            char *response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
            send_response(client, response_header);

            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL)
            {
                char entry_path[1028];
                snprintf(entry_path, sizeof(entry_path), "%s", entry->d_name);
                char entry_link[1028];
                if (entry->d_type == DT_DIR)
                {
                    snprintf(entry_link, sizeof(entry_link), "<a href=\"%s/\"> <b>%s/</b></a><br>", entry_path, entry_path);
                    strcat(entry_link, "<br>");
                }
                else
                {
                    snprintf(entry_link, sizeof(entry_link), "<a href=\"%s\"><i>%s/</i></a><br>", entry_path, entry_path);
                    strcat(entry_link, "<br>");
                }
                send_response(client, entry_link);
            }

            closedir(dir);
        }
        else
        {
            char *response_header = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\n";
            send_response(client, response_header);
            char *response_body = "<html><body><h1>Internal Server Error</h1></body></html>";
            send_response(client, response_body);
        }
    }
    else
    {
        FILE *file = fopen(path + 1, "rb");
        if (file != NULL)
        {
            char *file_extension = strrchr(path, '.');
            if (file_extension != NULL)
            {
                if (strcmp(file_extension, ".mp3") == 0)
                {
                    send_file(client, file, "audio/mp3");
                    fclose(file);
                    close(client);
                    return NULL;
                }
                else if (strcmp(file_extension, ".c") == 0 || strcmp(file_extension, ".cpp") == 0 || strcmp(file_extension, ".txt") == 0)
                {
                    send_file(client, file, "text/plain");
                    fclose(file);
                    close(client);
                    return NULL;
                }
                else if (strcmp(file_extension, ".jpg") == 0 || strcmp(file_extension, ".png") == 0)
                {
                    send_file(client, file, "image/jpeg");
                    fclose(file);
                    close(client);
                    return NULL;
                }
            }

            fclose(file);
        }

        DIR *dir = opendir(path + 1);
        if (dir != NULL)
        {
            char *response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
            send_response(client, response_header);

            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL)
            {
                char entry_path[256];
                snprintf(entry_path, sizeof(entry_path), "%s/%s", path + 1, entry->d_name);
                char entry_link[512];
                if (entry->d_type == DT_DIR)
                {
                    snprintf(entry_link, sizeof(entry_link), "<b href=\"%s/\">%s/</b><br>", entry_path, entry->d_name);
                    strcat(entry_link, "<br>");
                }
                else
                {
                    snprintf(entry_link, sizeof(entry_link), "<i href=\"%s\">%s</i><br>", entry_path, entry->d_name);
                    strcat(entry_link, "<br>");
                }
                send_response(client, entry_link);
            }

            closedir(dir);
        }
        else
        {
            char *response_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";
            send_response(client, response_header);
            char *response_body = "<html><body><h1>404 Not Found</h1></body></html>";
            send_response(client, response_body);
        }
    }

    close(client);
    return NULL;
}
