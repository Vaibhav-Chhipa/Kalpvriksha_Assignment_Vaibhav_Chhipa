#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define MAX_ARRAY_SIZE 100

struct SharedMemoryData
{
    int arraySize;
    int dataArray[MAX_ARRAY_SIZE];
};

unsigned int getIntegerInput()
{
    unsigned int number;
    char buffer[100];

    while (1)
    {
        if (fgets(buffer, sizeof(buffer), stdin))
        {
            buffer[strcspn(buffer, "\n")] = 0;
            if (sscanf(buffer, "%u", &number) == 1)
                return number;
            printf("Enter valid number!\n");
        }
    }
}

void bubbleSortArray(int *array, int size)
{
    for (int i = 0; i < size - 1; i++)
        for (int j = 0; j < size - i - 1; j++)
            if (array[j] > array[j + 1])
            {
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
}

void printArray(const int *array, int size)
{
    for (int i = 0; i < size; i++)
        printf("%d ", array[i]);
    printf("\n");
}

void childProcess(struct SharedMemoryData *sharedMemoryPtr)
{
    /* Validate size from shared memory */
    if (sharedMemoryPtr->arraySize <= 0 || sharedMemoryPtr->arraySize > MAX_ARRAY_SIZE)
    {
        fprintf(stderr, "Child: Invalid size in shared memory (%d)\n", sharedMemoryPtr->arraySize);
        return;
    }

    printf("\nChild Process (PID %d): Sorting data\n", getpid());

    /* Sort array in shared memory */
    bubbleSortArray(sharedMemoryPtr->dataArray, sharedMemoryPtr->arraySize);
}

void parentProcess(struct SharedMemoryData *sharedMemoryPtr, int *arrayData, int size)
{
    /* Write data to shared memory */
    sharedMemoryPtr->arraySize = size;

    for (int i = 0; i < size; i++)
    {
        sharedMemoryPtr->dataArray[i] = arrayData[i];
    }

    /* Wait for child and check exit status */
    wait(NULL);

    /* Read sorted data from shared memory */
    for (int i = 0; i < size; i++)
    {
        arrayData[i] = sharedMemoryPtr->dataArray[i];
    }

    printf("Parent Process (PID %d): Display sorted array\n", getpid());
    printArray(arrayData, size);
}

void cleanupResources(struct SharedMemoryData *sharedMemoryPtr, int sharedMemoryId, int *arrayData, bool isParent)
{
    /* Detach shared memory */
    if (sharedMemoryPtr && shmdt(sharedMemoryPtr) == -1)
    {
        perror("shmdt failed");
    }

    /* Only parent removes shared memory */
    if (isParent && sharedMemoryId != -1 &&
        shmctl(sharedMemoryId, IPC_RMID, NULL) == -1)
    {
        perror("shmctl failed");
    }

    free(arrayData);
}

int main()
{
    printf("Enter size of array: ");
    int size = getIntegerInput();

    if (size <= 0 || size > MAX_ARRAY_SIZE)
    {
        printf("Invalid array size. Must be between 1 and %d\n", MAX_ARRAY_SIZE);
        return 1;
    }

    int *arrayData = malloc(size * sizeof(int));
    if (!arrayData)
    {
        perror("malloc failed");
        return 1;
    }

    /* Read array elements from user */
    for (int i = 0; i < size; i++)
    {
        printf("Enter element %d: ", i + 1);
        arrayData[i] = getIntegerInput();
    }

    printf("Parent Process (PID %d): Display original array\n", getpid());
    printArray(arrayData, size);

    /* Create unique key for shared memory */
    key_t key = ftok(".", 66);
    if (key == -1)
    {
        perror("ftok failed");
        free(arrayData);
        return 1;
    }

    /* Create shared memory segment */
    int sharedMemoryId =
        shmget(key, sizeof(struct SharedMemoryData), IPC_CREAT | 0666);
    if (sharedMemoryId == -1)
    {
        perror("shmget failed");
        free(arrayData);
        return 1;
    }

    /* Attach shared memory segment */
    struct SharedMemoryData *sharedMemoryPtr =
        (struct SharedMemoryData *)shmat(sharedMemoryId, NULL, 0);

    if (sharedMemoryPtr == (void *)-1)
    {
        perror("shmat failed");
        cleanupResources(NULL, sharedMemoryId, arrayData, 1);
        return 1;
    }

    /* Create child process */
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        cleanupResources(sharedMemoryPtr, sharedMemoryId, arrayData, 1);
        return 1;
    }
    else if (pid == 0)
    {
        /* Child process: read from shared memory, sort, write back */
        childProcess(sharedMemoryPtr);
        cleanupResources(sharedMemoryPtr, -1, arrayData, 0);
        return 0;
    }
    else
    {
        /* Parent process: write to shared memory, wait, read sorted data */
        parentProcess(sharedMemoryPtr, arrayData, size);
        cleanupResources(sharedMemoryPtr, sharedMemoryId, arrayData, 1);
    }

    return 0;
}