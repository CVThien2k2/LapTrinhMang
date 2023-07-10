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
#include <stdbool.h>
#include <ctype.h>


#define BUFFER_SIZE 256

char Topic[50];
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
bool checkName(char *name)
{
    for (int i = 0; i < strlen(name); i++)
    {
        if (!isalnum(name[i]))
        {
            return false;
        }
    }
    return true;
}
void *handle_client(void *arg)
{
    int client = *(int *)arg;
    char buf[BUFFER_SIZE];
    char nameclient[50];
    while (1)
    {

        memset(buf, 0, sizeof(buf));
        int rcv = recv(client, buf, sizeof(buf), 0);
        if (rcv <= 0)
        {
            break;
        }
        if (strcmp(buf, "QUIT\n") == 0)
        {
            send(client, "100 OK\r\n", strlen("100 OK\r\n"), 0);
            break;
        }
        char first[50];
        memset(first, 0, sizeof(first));
        char last[50];
        memset(last, 0, sizeof(last));

        // Tìm vị trí khoảng trắng đầu tiên
        char *spacePosition = strchr(buf, ' ');

        if (spacePosition != NULL)
        {
            // Tính toán độ dài của cụm đầu tiên
            int firstClusterLength = spacePosition - buf;

            // Sao chép cụm đầu tiên vào biến firstCluster
            strncpy(first, buf, firstClusterLength);
            first[firstClusterLength] = '\0';

            // Sao chép chuỗi còn lại vào biến remainingString
            strcpy(last, spacePosition + 1);
            last[strlen(last) - 1] = '\0';
        }
        if (strcmp(first, "JOIN") == 0)
        {
            if (checkName(last))
            {
                struct node *current = head;
                int count = 0;
                while (current != NULL)
                {
                    if (strcmp(current->name, last) == 0)
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
                    createUser(last, client);
                    send(client, "100 OK\r\n", strlen("100 OK\r\n"), 0);
                    strcpy(nameclient, last);
                    struct node *current = head;
                    while (current != NULL)
                    {
                        if (current->numclient != client)
                        {
                            send(current->numclient, buf, strlen(buf), 0);
                        }
                        current = current->next;
                    }
                }
            }
            else{
                send(client, "201 INVALID NICK NAME\r\n", strlen("201 INVALID NICK NAME\r\n"), 0);
            }
        }
        else if (strcmp(first, "MSG") == 0)
        {

            char full_msg[500];
            int msg_len = snprintf(full_msg, sizeof(full_msg), "MSG %s %s\r\n", nameclient, last);
            struct node *current = head;
            current = head;
            send(client, "100 OK\r\n", strlen("100 OK\r\n"), 0);
            while (current != NULL)
            {
                if (current->numclient != client)
                {
                    send(current->numclient, full_msg, strlen(full_msg), 0);
                }
                current = current->next;
            }
        }
        else if (strcmp(first, "PMSG") == 0)
        {
            char name1[50];
            char msg[50];

            // Tìm vị trí khoảng trắng đầu tiên
            char *spacePosition1 = strchr(last, ' ');

            if (spacePosition1 != NULL)
            {
                // Tính toán độ dài của cụm đầu tiên
                int names = spacePosition1 - last;

                // Sao chép cụm đầu tiên vào biến firstCluster
                strncpy(name1, last, names);
                name1[names] = '\0';

                // Sao chép chuỗi còn lại vào biến remainingString
                strcpy(msg, spacePosition1 + 1);
            }
            struct node *current = head;
            int count = 0;
            while (current != NULL)
            {
                if (strcmp(current->name, name1)==0)
                {
                    send(client, "100 OK\r\n", strlen("100 OK\r\n"), 0);
                    char full_msg[500];
                    int msg_len = snprintf(full_msg, sizeof(full_msg), "PMSG %s %s\r\n", nameclient, msg);
                    send(current->numclient, full_msg, strlen(full_msg), 0);
                    count++;
                    break;
                }
                current = current->next;
            }
            if (count == 0)
            {
                send(client, "202 UNKNOWN NICKNAME\r\n", strlen("202 UNKNOWN NICKNAME\r\n"), 0);
            }
        }
        else if (strcmp(first, "KICK") == 0)
        {
            if (key->numclient != client)
            {
                send(client, "203 DENIED\r\n", strlen("203 DENIED\r\n"), 0);
            }
            else
            {
                struct node *current = head;
                int count = 0;
                while (current != NULL)
                {
                    if (strcmp(current->name, last)==0)
                    {
                        send(client, "100 OK\r\n", strlen("100 OK\r\n"), 0);
                        current->prev->next = current->next;
                        current->next->prev = current->prev;
                        count++;
                        close(current->numclient);
                        break;
                    }
                    current = current->next;
                }
                if (count == 0)
                {
                    send(client, "202 UNKNOWN NICKNAME\r\n", strlen("202 UNKNOWN NICKNAME\r\n"), 0);
                }
            }
        }
        else if (strcmp(first, "OP") == 0)
        {
            if (key->numclient != client)
            {
                send(client, "203 DENIED\r\n", strlen("203 DENIED\r\n"), 0);
            }
            else
            {
                struct node *current = head;
                int count = 0;
                while (current != NULL)
                {
                    if (strcmp(current->name, last))
                    {
                        key = current;
                        send(client, "100 OK\r\n", strlen("100 OK\r\n"), 0);
                        count++;
                        break;
                    }
                    current = current->next;
                }
                if (count == 0)
                {
                    send(client, "202 UNKNOWN NICKNAME\r\n", strlen("202 UNKNOWN NICKNAME\r\n"), 0);
                }
            }
        }
        else if (strcmp(first, "TOPIC") == 0)
        {
            if (key->numclient != client)
            {
                send(client, "203 DENIED\r\n", strlen("203 DENIED\r\n"), 0);
            }
            else
            {
                send(client, "100 OK\r\n", strlen("100 OK\r\n"), 0);
                strcpy(Topic, last);
            }
        }
        else
        {
            send(client, "999 UNKNOWN ERROR\r\n", strlen("999 UNKNOWN ERROR\r\n"), 0);
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