#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>

#define MAX_CLIENTS 64
#define BUFFER_SIZE 256

int clients[MAX_CLIENTS];
char nameClients[MAX_CLIENTS][50];
int num_clients = 0;

struct node
{
    char name[50];
    int numclient;
    struct node *next;
    struct node *prev;
};
struct node *head = NULL;
struct node *key = NULL;

void createUser(const char *name, int numclient)
{
    // tao mot user
    struct node *user = (struct node *)malloc(sizeof(struct node));

    strcpy(user->name, name);
    user->numclient = numclient;

    if (head == NULL)
    {
        head = user;
        user->next = NULL;
        user->prev = NULL;
        key = user;
    }
    else
    {
        struct node *current = head;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = user;
        user->prev = current;
        user->next = NULL;
    }
}
void *handle_client(void *arg)
{
    int client = *(int *)arg;
    char buf[BUFFER_SIZE];

    while (1)
    {
        memset(buf, 0, sizeof(buf));
        int rcv = recv(client, buf, sizeof(buf), 0);
        if (rcv <= 0)
        {
            break;
        }
        char firstCluster[50];
        char remainingString[50];

        // Tìm vị trí khoảng trắng đầu tiên
        char *spacePosition = strchr(buf, ' ');

        if (spacePosition != NULL)
        {
            // Tính toán độ dài của cụm đầu tiên
            int firstClusterLength = spacePosition - buf;

            // Sao chép cụm đầu tiên vào biến firstCluster
            strncpy(firstCluster, buf, firstClusterLength);
            firstCluster[firstClusterLength] = '\0';

            // Sao chép chuỗi còn lại vào biến remainingString
            strcpy(remainingString, spacePosition + 1);
        }
        if (strcmp(firstCluster, "JOIN") == 0)
        {
            struct node *current = head;
            int count = 0;
            while (current != NULL)
            {
                if (strcmp(current->name, remainingString) == 0)
                {
                    count++;
                }
                current = current->next;
            }
            if (count != 0)
            {
                send(client, "200 NICKNAME IN USE\r\n", strlen("200 NICKNAME IN USE\r\n"), 0);
            }
            else
            {
                createUser(remainingString, client);
                send(client, "100 OK\r\n", strlen("100 OK\r\n"), 0);
                struct node *current = head;

                while (current != NULL)
                {
                    if (current->numclient != client)
                    {
                        send(client, "100 OK\r\n", strlen("100 OK\r\n"), 0);
                    }
                    current = current->next;
                }
            }
        }
    }

    close(client);
    pthread_exit(NULL);
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

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, &client) != 0)
        {
            perror("pthread_create() failed");
            close(client);
            continue;
        }

        pthread_detach(thread);
    }

    close(listener);
    return 0;
}