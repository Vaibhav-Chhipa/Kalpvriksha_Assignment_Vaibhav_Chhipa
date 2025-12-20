#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#define FILENAME "data.txt"

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

bool writeInFile(const int *arr, int size)
{
    FILE *file = fopen(FILENAME, "w");
    if (!file)
    {
        printf("File open failed\n");
        return false;
    }

    for (int i = 0; i < size; i++)
        fprintf(file, "%d ", arr[i]);

    fclose(file);
    return true;
}

bool readFromFile(int *arr, int size)
{
    FILE *file = fopen(FILENAME, "r");
    if (!file)
    {
        printf("File open failed\n");
        return false;
    }

    for (int i = 0; i < size; i++)
    {
        if (fscanf(file, "%d", &arr[i]) != 1)
        {
            fclose(file);
            return false;
        }
    }

    fclose(file);
    return true;
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

void childProcess(int *arr, int size)
{
    printf("\nChild Process (PID %d): Sorting data\n", getpid());

    if (readFromFile(arr, size))
    {
        bubbleSort(arr, size);
        writeInFile(arr, size);
    }

    free(arr);
}

void parentProcess(int *arr, int size)
{
    wait(NULL);

    printf("\nParent Process (PID %d): Display sorted array\n", getpid());

    if (readFromFile(arr, size))
        printArray(arr, size);

    remove(FILENAME);
    free(arr);
}

int main()
{
    printf("Enter size of array: ");
    int size = getIntInput();

    int *arr = malloc(size * sizeof(int));
    if (!arr)
    {
        printf("Memory allocation failed\n");
        return 1;
    }

    for (int i = 0; i < size; i++)
    {
        printf("Enter element %d: ", i + 1);
        arr[i] = getIntInput();
    }

    writeInFile(arr, size);

    printf("\nParent Process (PID %d): Original array\n", getpid());
    printArray(arr, size);

    fflush(stdout);

    pid_t pid = fork();

    if (pid < 0)
    {
        printf("Fork failed\n");
        free(arr);
        return 1;
    }
    else if (pid == 0)
    {
        childProcess(arr, size);
        return 0;
    }
    else
    {
        parentProcess(arr, size);
    }

    return 0;
}
