#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define MAX_SIZE 100

struct message
{
    long msgType;
    int size;
    int arr[MAX_SIZE];
};

unsigned int getIntInput()
{
    unsigned int num;
    char buffer[100];

    while (1)
    {
        if (fgets(buffer, sizeof(buffer), stdin))
        {
            buffer[strcspn(buffer, "\n")] = 0;
            if (sscanf(buffer, "%u", &num) == 1)
                return num;
            printf("Enter valid number!\n");
        }
    }
}

void bubbleSort(int *arr, int size)
{
    for (int i = 0; i < size - 1; i++)
    {
        for (int j = 0; j < size - i - 1; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void printArray(const int *arr, int size)
{
    for (int i = 0; i < size; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

void childProcess(int msgid)
{
    struct message msg;

    /* Receive unsorted data from parent (msgType=1) */
    if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 1, 0) == -1)
    {
        perror("Child msgrcv failed");
        return;
    }

    printf("\nChild Process (PID %d): Sorting data\n", getpid());

    bubbleSort(msg.arr, msg.size);

    /* Send sorted data back to parent (msgType=2) */
    msg.msgType = 2;
    if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1)
    {
        perror("Child msgsnd failed");
    }
}

void parentProcess(int msgid, int *arr, int size)
{
    struct message msg;

    /* Prepare message with unsorted data */
    msg.msgType = 1;
    msg.size = size;

    for (int i = 0; i < size; i++)
        msg.arr[i] = arr[i];

    /* Send unsorted data to child process */
    if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1)
    {
        perror("Parent msgsnd failed");
        return;
    }

    /* Receive sorted data from child (msgType=2) */
    if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 2, 0) == -1)
    {
        perror("Parent msgrcv failed");
        return;
    }

    printf("\nParent Process (PID %d): Display sorted array\n", getpid());
    printArray(msg.arr, msg.size);

    /* Clean up message queue */
    if (msgctl(msgid, IPC_RMID, NULL) == -1)
    {
        perror("msgctl failed");
    }
}

int main()
{
    printf("Enter size of array: ");
    int size = getIntInput();

    if (size == 0)
    {
        printf("Array size must be greater than 0\n");
        return 1;
    }

    if (size > MAX_SIZE)
    {
        printf("Max allowed size is %d\n", MAX_SIZE);
        return 1;
    }

    int *arr = malloc(size * sizeof(int));
    if (!arr)
    {
        printf("Memory allocation failed\n");
        return 1;
    }

    /* Read array elements from user */
    for (int i = 0; i < size; i++)
    {
        printf("Enter element %d: ", i + 1);
       arr[i] = getIntInput();
    }

    printf("Parent Process (PID %d): Display original array\n", getpid());
    printArray(arr, size);

    /* Create unique key for message queue */
    key_t key = ftok(".", 65);
    if (key == -1)
    {
        perror("ftok failed");
        free(arr);
        return 1;
    }

    /* Create message queue */
    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1)
    {
        perror("Message queue creation failed");
        free(arr);
        return 1;
    }

    /* Create child process */
    pid_t pid = fork();

    if (pid < 0)
    {
        printf("Fork failed\n");
        msgctl(msgid, IPC_RMID, NULL);
        free(arr);
        return 1;
    }
    else if (pid == 0)
    {
        /* Child process: receive, sort, and send back */
        childProcess(msgid);
        free(arr);
        return 0;
    }
    else
    {
        /* Parent process: send, wait, receive, and display */
        parentProcess(msgid, arr, size);

        int status;
        wait(&status);

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        {
            fprintf(stderr, "Child process exited with error\n");
        }
    }

    free(arr);
    return 0;
}