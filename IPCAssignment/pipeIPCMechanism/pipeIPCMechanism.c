#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_SIZE 100

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

void childProcess(int readPipe, int writePipe)
{
    int size;
    int arr[MAX_SIZE];

    /* Read size */
    if (read(readPipe, &size, sizeof(int)) <= 0)
    {
        perror("Child read size failed");
        return;
    }

    /* Read array */
    if (read(readPipe, arr, size * sizeof(int)) <= 0)
    {
        perror("Child read array failed");
        return;
    }

    printf("\nChild Process (PID %d): Sorting data\n", getpid());

    bubbleSort(arr, size);

    if (write(writePipe, arr, size * sizeof(int)) <= 0)
    {
        perror("Child write failed");
    }
}

void parentProcess(int writePipe, int readPipe, int *arr, int size)
{
    /* Send size */
    if (write(writePipe, &size, sizeof(int)) <= 0)
    {
        perror("Parent write size failed");
        return;
    }

    /* Send array */
    if (write(writePipe, arr, size * sizeof(int)) <= 0)
    {
        perror("Parent write array failed");
        return;
    }

    /* Wait for child to sort */
    wait(NULL);

    /* Read sorted array */
    if (read(readPipe, arr, size * sizeof(int)) <= 0)
    {
        perror("Parent read failed");
        return;
    }

    printf("\nParent Process (PID %d): Display sorted array\n", getpid());
    printArray(arr, size);
}

int main()
{
    printf("Enter size of array: ");
    int size = getIntInput();

    if (size <= 0 || size > MAX_SIZE)
    {
        printf("Invalid array size\n");
        return 1;
    }

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

    printf("Parent Process (PID %d): Display original array\n", getpid());
    printArray(arr, size);

    int pipe1[2]; // Parent -> Child (pipe1[1] write, pipe1[0] read)
    int pipe2[2]; // Child -> Parent (pipe2[1] write, pipe2[0] read)

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1)
    {
        perror("Pipe creation failed");
        free(arr);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("Fork failed");
        free(arr);
        return 1;
    }
    else if (pid == 0)
    {
        /* Child */
        close(pipe1[1]); // close write end
        close(pipe2[0]); // close read end

        childProcess(pipe1[0], pipe2[1]);

        close(pipe1[0]);
        close(pipe2[1]);
        free(arr);
        return 0;
    }
    else
    {
        /* Parent */
        close(pipe1[0]); // close read end
        close(pipe2[1]); // close write end

        parentProcess(pipe1[1], pipe2[0], arr, size);

        close(pipe1[1]);
        close(pipe2[0]);
    }

    free(arr);
    return 0;
}
