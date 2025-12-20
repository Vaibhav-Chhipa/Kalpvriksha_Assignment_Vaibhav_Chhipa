#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>

#define PORT 8080
#define ADDRESS "127.0.0.1"
#define DB_FILE "../resource/accountDB.txt"

pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;

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
        if (fscanf(file, "%u", &balance) != 1)
        {
            balance = 0;
        }
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
            printf("Account database created with initial balance: %u\n", balance);
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
    char response[100];
    
    printf("New client connected. Thread started.\n");
    
    while (1)
    {
        memset(&transaction, 0, sizeof(transaction));
        int bytesRead = recv(clientSocket, &transaction, sizeof(transaction), 0);
        
        if (bytesRead != sizeof(transaction))
        {
            if (bytesRead == 0)
            {
                printf("Client disconnected.\n");
            }
            else
            {
                printf("Invalid data received from client.\n");
            }
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
                    sprintf(response, "Withdrawal failed! Amount must be greater than zero.");
                }
                else if (transaction.amount > balance)
                {
                    sprintf(response, "Withdrawal failed! Insufficient balance. Current balance: %u", balance);
                }
                else
                {
                    balance -= transaction.amount;
                    writeBalance(balance);
                    sprintf(response, "Withdrawal successful! Amount withdrawn: %u, New balance: %u", 
                            transaction.amount, balance);
                    printf("Withdrawal processed: %u, New balance: %u\n", transaction.amount, balance);
                }
                break;
                
            case DEPOSIT:
                if (transaction.amount == 0)
                {
                    sprintf(response, "Deposit failed! Amount must be greater than zero.");
                }
                else
                {
                    balance += transaction.amount;
                    writeBalance(balance);
                    sprintf(response, "Deposit successful! Amount deposited: %u, New balance: %u", 
                            transaction.amount, balance);
                    printf("Deposit processed: %u, New balance: %u\n", transaction.amount, balance);
                }
                break;
                
            case DISPLAY:
                sprintf(response, "Current balance: %u", balance);
                printf("Balance inquiry: %u\n", balance);
                break;
                
            default:
                sprintf(response, "Invalid operation requested.");
                printf("Invalid operation received.\n");
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
    int serverSocket;
    int opt = 1;
    
    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        return 1;
    }
    
    // Set socket options
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        close(serverSocket);
        return 1;
    }
    
    // Setup server address
    struct sockaddr_in serverAddr;
    int addrLen = sizeof(serverAddr);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ADDRESS);
    serverAddr.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Bind failed");
        close(serverSocket);
        return 1;
    }
    
    // Listen for connections
    if (listen(serverSocket, 5) < 0)
    {
        perror("Listen failed");
        close(serverSocket);
        return 1;
    }
    
    printf("========================================\n");
    printf("  ATM Server Started on Port %d\n", PORT);
    printf("========================================\n");
    printf("Waiting for client connections...\n\n");
    
    while (1)
    {
        int *clientSocket = malloc(sizeof(int));
        if (clientSocket == NULL)
        {
            perror("malloc");
            continue;
        }
        
        if ((*clientSocket = accept(serverSocket, (struct sockaddr *)&serverAddr, (socklen_t *)&addrLen)) < 0)
        {
            perror("Accept failed");
            free(clientSocket);
            continue;
        }
        
        printf("Connection established with client.\n");
        
        pthread_t threadId;
        if (pthread_create(&threadId, NULL, handleClient, clientSocket) != 0)
        {
            perror("Thread creation failed");
            close(*clientSocket);
            free(clientSocket);
            continue;
        }
        
        pthread_detach(threadId);
    }
    
    close(serverSocket);
    pthread_mutex_destroy(&fileMutex);
    
    return 0;
}