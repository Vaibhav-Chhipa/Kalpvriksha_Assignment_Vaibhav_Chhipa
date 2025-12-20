#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define PORT 8080
#define ADDRESS "127.0.0.1"

typedef enum commands
{
    WITHDRAW,
    DEPOSIT,
    DISPLAY,
    EXIT,
    INVALID
} commands;

typedef struct ATMTransaction
{
    enum commands operation;
    unsigned int amount;
} ATMTransaction;

void displayMenu()
{
    printf("\n========================================\n");
    printf("        ATM TRANSACTION SYSTEM\n");
    printf("========================================\n");
    printf("1. Withdraw Amount\n");
    printf("2. Deposit Amount\n");
    printf("3. Display Balance\n");
    printf("4. Exit\n");
    printf("========================================\n");
}

unsigned int getUnsignedInput(const char *prompt)
{
    unsigned int value;
    char buffer[100];
    
    while (1)
    {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            buffer[strcspn(buffer, "\n")] = 0;
            
            if (sscanf(buffer, "%u", &value) == 1)
            {
                return value;
            }
            else
            {
                printf("Invalid input!\n");
            }
        }
        else
        {
            printf("Error reading input.\n");
        }
    }
}

enum commands mapChoiceToCommands(unsigned int choice)
{
    switch (choice)
    {
        case 1:
        return WITHDRAW;
        case 2:
        return DEPOSIT;
        case 3:
        return DISPLAY;
        case 4:
        return EXIT;
        default:
        return INVALID;
    }
}

bool processUserInput(int clientSocket)
{
    ATMTransaction transaction = {0};
    
    displayMenu();
    
    unsigned int choice = getUnsignedInput("Enter your choice: ");
    transaction.operation = mapChoiceToCommands(choice);
    
    if (transaction.operation == INVALID)
    {
        printf("\nInvalid choice! Please select 1-4.\n");
        return true;
    }
    
    if (transaction.operation == WITHDRAW || transaction.operation == DEPOSIT)
    {
        transaction.amount = getUnsignedInput("Enter amount: ");
    }
    else
    {
        transaction.amount = 0;
    }
    
    // Send transaction to server
    if (send(clientSocket, &transaction, sizeof(transaction), 0) <= 0)
    {
        perror("send");
        return false;
    }
    
    // If exit, return false to terminate
    if (transaction.operation == EXIT)
    {
        printf("\nThank you for using our ATM service!\n");
        return false;
    }
    
    return true;
}


int main()
{
    int clientSocket;
    
    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        return 1;
    }
    
    // Setup server address
    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    
    // Convert IP address
    if (inet_pton(AF_INET, ADDRESS, &serverAddr.sin_addr) <= 0)
    {
        printf("Invalid address!\n");
        close(clientSocket);
        return 1;
    }
    
    // Connect to server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Connection failed");
        printf("Make sure the server is running on %s:%d\n", ADDRESS, PORT);
        close(clientSocket);
        return 1;
    }
    
    printf("\n========================================\n");
    printf("  Connected to ATM Server Successfully!\n");
    printf("========================================\n");
    
    while (1)
    {
        if (!processUserInput(clientSocket))
        {
            close(clientSocket);
            return 0;
        }
        
        char response[256] = {0};
        int bytesRead = recv(clientSocket, response, sizeof(response) - 1, 0);
        
        if (bytesRead <= 0)
        {
            if (bytesRead == 0)
            {
                printf("Server disconnected.\n");
            }
            else
            {
                perror("recv");
            }
            break;
        }
        
        response[bytesRead] = '\0';
        printf("\nServer Response: %s\n", response);
    }
    
    close(clientSocket);
    return 0;
}