#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <signal.h>

#define PORT 8080
#define ADDRESS "127.0.0.1"
#define DB_FILE "../resource/accountDB.txt"

pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;
int serverSocket;

void signalHandler(int sig)
{
    printf("\n\nShutting down server gracefully...\n");
    close(serverSocket);
    pthread_mutex_destroy(&fileMutex);
    exit(0);
}

typedef enum command
{
    WITHDRAW,
    DEPOSIT,
    DISPLAY,
    EXIT
} command;

typedef struct ATMTransaction
{
    enum command operation;
    unsigned int amount;
} ATMTransaction;

unsigned int readBalance()
{
    FILE *file = fopen(DB_FILE, "r");
    unsigned int balance = 0;

    if (file)
    {
        fscanf(file, "%u", &balance);
        fclose(file);
    }
    else
    {
        file = fopen(DB_FILE, "w");
        if (file)
        {
            balance = 500000;
            fprintf(file, "%u\n", balance);
            fclose(file);
            printf("Account DB created with initial balance: %u\n", balance);
        }
        else
        {
            perror("fopen");
        }
    }
    return balance;
}

void writeBalance(unsigned int balance)
{
    FILE *file = fopen(DB_FILE, "w");
    if (file)
    {
        fprintf(file, "%u\n", balance);
        fclose(file);
    }
    else
    {
        perror("fopen");
    }
}

void *handleClient(void *arg)
{
    int clientSocket = *(int *)arg;
    free(arg);

    ATMTransaction transaction;
    char response[128];

    printf("New client connected. Thread started.\n");

    while (1)
    {
        memset(&transaction, 0, sizeof(transaction));

        int bytesRead = recv(clientSocket, &transaction, sizeof(transaction), 0);
        if (bytesRead <= 0)
        {
            if (bytesRead == 0)
                printf("Client disconnected.\n");
            else
                perror("recv");
            break;
        }

        if (transaction.operation == EXIT)
        {
            printf("Client requested exit.\n");
            break;
        }

        pthread_mutex_lock(&fileMutex);

        unsigned int balance = readBalance();

        switch (transaction.operation)
        {
            case WITHDRAW:
                if (transaction.amount == 0)
                {
                    sprintf(response, "Withdrawal failed! Amount must be > 0");
                }
                else if (transaction.amount > balance)
                {
                    sprintf(response, "Withdrawal failed! Balance: %u", balance);
                }
                else
                {
                    balance -= transaction.amount;
                    writeBalance(balance);
                    sprintf(response, "Withdrawal successful! New balance: %u", balance);
                }
                break;

            case DEPOSIT:
                if (transaction.amount == 0)
                {
                    sprintf(response, "Deposit failed! Amount must be > 0");
                }
                else
                {
                    balance += transaction.amount;
                    writeBalance(balance);
                    sprintf(response, "Deposit successful! New balance: %u", balance);
                }
                break;

            case DISPLAY:
                sprintf(response, "Current balance: %u", balance);
                break;

            default:
                sprintf(response, "Invalid operation");
                break;
        }

        pthread_mutex_unlock(&fileMutex);

        if (send(clientSocket, response, strlen(response) + 1, 0) <= 0)
        {
            perror("send");
            break;
        }
    }

    close(clientSocket);
    return NULL;
}

int main()
{
    int opt = 1;

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        {
            perror("setsockopt failed");
            close(serverSocket);
            return 1;   
        }
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ADDRESS);
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("bind");
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 5) < 0)
    {
        perror("listen");
        close(serverSocket);
        return 1;
    }

    printf("========================================\n");
    printf("  ATM Server Started on Port %d\n", PORT);
    printf("========================================\n");
    printf("Waiting for client connections...\n\n");

    while (1)
    {
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);

        int *clientSocket = malloc(sizeof(int));
        if (!clientSocket)
        {
            perror("malloc");
            continue;
        }

        *clientSocket = accept(serverSocket,
                               (struct sockaddr *)&clientAddr,
                               &addrLen);
        if (*clientSocket < 0)
        {
            perror("accept");
            free(clientSocket);
            continue;
        }

        pthread_t threadId;
        if (pthread_create(&threadId, NULL, handleClient, clientSocket) != 0)
        {
            perror("pthread_create");
            close(*clientSocket);
            free(clientSocket);
            continue;
        }

        pthread_detach(threadId);
    }

    return 0;
}
 