#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PROCESSES 1000
#define MAX_NAME 50
#define MAX_LINE 200

typedef enum
{
    READY,
    RUNNING,
    WAITING,
    TERMINATED
} State;

typedef struct PCB
{
    int pid;
    char *name;
    int burst_time;
    int io_start_time;
    int io_duration;
    int remaining_burst;
    int remaining_io;
    int execution_time;
    int waiting_time;
    int turnaround_time;
    int completion_time;
    State state;
    int killed;
    int io_just_started;
    struct PCB *next_in_hash;
} PCB;

typedef struct Node
{
    int pid;
    struct Node *next;
} Node;

typedef struct Queue
{
    Node *front;
    Node *rear;
    int size;
} Queue;

typedef struct KillEvent
{
    int pid;
    int kill_time;
    struct KillEvent *next;
} KillEvent;

int hashFunction(int pid)
{
    if (pid < 0)
    {
        pid = -pid;
    }
    return pid % MAX_PROCESSES;
}

PCB *getPCB(PCB *hashmap[], int pid)
{
    int index = hashFunction(pid);
    PCB *current = hashmap[index];

    while (current != NULL)
    {
        if (current->pid == pid)
        {
            return current;
        }
        current = current->next_in_hash;
    }

    return NULL;
}

int insertPCB(PCB *hashmap[], PCB *pcb)
{
    if (pcb == NULL)
        return 0;
    if (getPCB(hashmap, pcb->pid) != NULL)
    {
        printf("Error: PID %d already exists!\n", pcb->pid);
        return 0;
    }

    int index = hashFunction(pcb->pid);
    pcb->next_in_hash = hashmap[index];
    hashmap[index] = pcb;

    return 1;
}

void freePCBHashmap(PCB *hashmap[])
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        PCB *current = hashmap[i];
        while (current != NULL)
        {
            PCB *temp = current;
            current = current->next_in_hash;

            if (temp->name != NULL)
            {
                free(temp->name);
            }
            free(temp);
        }
        hashmap[i] = NULL;
    }
}

void freeQueue(Queue *q)
{
    if (q == NULL)
        return;

    while (q->front != NULL)
    {
        Node *temp = q->front;
        q->front = q->front->next;
        free(temp);
    }
    free(q);
}

void freeKillEvents(KillEvent **head)
{
    if (head == NULL)
        return;

    while (*head != NULL)
    {
        KillEvent *temp = *head;
        *head = (*head)->next;
        free(temp);
    }
}

void cleanupAll(PCB *hashmap[], Queue *readyQ, Queue *waitingQ, Queue *terminatedQ, KillEvent **killHead)
{
    freePCBHashmap(hashmap);
    freeQueue(readyQ);
    freeQueue(waitingQ);
    freeQueue(terminatedQ);
    freeKillEvents(killHead);
}

Queue *createQueue()
{
    Queue *q = (Queue *)malloc(sizeof(Queue));
    if (q == NULL)
    {
        printf("Memory allocation failed for Queue!\n");
        return NULL;
    }

    q->front = q->rear = NULL;
    q->size = 0;
    return q;
}

Node *createNode(int pid)
{
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL)
    {
        printf("Memory allocation failed for Node!\n");
        return NULL;
    }
    node->pid = pid;
    node->next = NULL;
    return node;
}

int enqueue(Queue *q, int pid)
{
    Node *node = createNode(pid);
    if (node == NULL)
        return 0;

    if (q->rear == NULL)
    {
        q->front = q->rear = node;
    }
    else
    {
        q->rear->next = node;
        q->rear = node;
    }
    q->size++;
    return 1;
}

int dequeue(Queue *q)
{
    if (q->front == NULL)
        return -1;
    Node *temp = q->front;
    int pid = temp->pid;
    q->front = q->front->next;
    if (q->front == NULL)
        q->rear = NULL;
    free(temp);
    q->size--;
    return pid;
}

int isQueueEmpty(Queue *q)
{
    return q == NULL || q->front == NULL;
}

int getValidInteger()
{
    int num;
    while (1)
    {
        if (scanf("%d", &num) != 1)
        {
            printf("Invalid input, enter again: ");
            while (getchar() != '\n')
                ;
        }
        else
        {
            return num;
        }
    }
}

char *stringCopy(const char *src)
{
    if (src == NULL)
        return NULL;

    size_t len = strlen(src);
    char *dest = (char *)malloc(len + 1);
    if (dest == NULL)
    {
        printf("Memory allocation failed for string!\n");
        return NULL;
    }
    strcpy(dest, src);
    return dest;
}

KillEvent *createKillEvent(int pid, int time)
{
    KillEvent *event = (KillEvent *)malloc(sizeof(KillEvent));
    if (event == NULL)
    {
        printf("Memory allocation failed for KillEvent!\n");
        return NULL;
    }
    event->pid = pid;
    event->kill_time = time;
    event->next = NULL;
    return event;
}

int addKillEvent(KillEvent **head, int pid, int time)
{
    KillEvent *event = createKillEvent(pid, time);
    if (event == NULL)
        return 0;

    if (*head == NULL || (*head)->kill_time > time)
    {
        event->next = *head;
        *head = event;
        return 1;
    }
    KillEvent *curr = *head;
    while (curr->next != NULL && curr->next->kill_time <= time)
    {
        curr = curr->next;
    }
    event->next = curr->next;
    curr->next = event;
    return 1;
}

void processKillEvents(KillEvent **head, PCB *hashmap[], int clock)
{
    while (*head != NULL && (*head)->kill_time == clock)
    {
        int pid = (*head)->pid;
        PCB *pcb = getPCB(hashmap, pid);
        if (pcb != NULL && !pcb->killed && pcb->state != TERMINATED)
        {
            pcb->killed = 1;
            pcb->state = TERMINATED;
            pcb->completion_time = clock;
        }
        KillEvent *temp = *head;
        *head = (*head)->next;
        free(temp);
    }
}

void removePCBFromHashmap(PCB *hashmap[], int pid)
{
    int index = hashFunction(pid);
    PCB *curr = hashmap[index], *prev = NULL;

    while (curr != NULL && curr->pid != pid)
    {
        prev = curr;
        curr = curr->next_in_hash;
    }

    if (curr != NULL)
    {
        if (prev == NULL)
            hashmap[index] = curr->next_in_hash;
        else
            prev->next_in_hash = curr->next_in_hash;
    }
}

int initializePCB(PCB *hashmap[], Queue *readyQueue, int pid, char *name, int burst, int io_Start, int io_Duration)
{
    PCB *pcb = (PCB *)malloc(sizeof(PCB));
    if (pcb == NULL)
    {
        printf("Memory allocation failed for PCB!\n");
        return 0;
    }

    pcb->name = stringCopy(name);
    if (pcb->name == NULL)
    {
        free(pcb);
        return 0;
    }

    pcb->pid = pid;
    pcb->burst_time = burst;
    pcb->remaining_burst = burst;
    pcb->io_start_time = io_Start;
    pcb->io_duration = io_Duration;
    pcb->remaining_io = io_Duration;
    pcb->execution_time = 0;
    pcb->turnaround_time = 0;
    pcb->waiting_time = 0;
    pcb->completion_time = 0;
    pcb->state = READY;
    pcb->killed = 0;
    pcb->io_just_started = 0;
    pcb->next_in_hash = NULL;

    if (!insertPCB(hashmap, pcb))
    {
        free(pcb->name);
        free(pcb);
        return 0;
    }

    if (!enqueue(readyQueue, pid))
    {
        removePCBFromHashmap(hashmap, pid);
        free(pcb->name);
        free(pcb);
        return 0;
    }

    return 1;
}

void updateWaitingProcesses(Queue *waitingQueue, Queue *readyQueue, PCB *hashmap[])
{
    if (waitingQueue == NULL)
        return;
    Node *curr = waitingQueue->front;
    Node *prev = NULL;

    while (curr != NULL)
    {
        PCB *pcb = getPCB(hashmap, curr->pid);
        Node *next = curr->next;

        if (!pcb->io_just_started)
        {
            if (pcb->remaining_io > 0)
            {
                pcb->remaining_io--;
                if (pcb->remaining_io == 0)
                {
                    if (!pcb->killed)
                    {
                        pcb->state = READY;
                        enqueue(readyQueue, pcb->pid);
                    }

                    if (prev == NULL)
                    {
                        waitingQueue->front = next;
                    }
                    else
                    {
                        prev->next = next;
                    }
                    if (next == NULL)
                        waitingQueue->rear = prev;

                    free(curr);
                    waitingQueue->size--;
                    curr = next;
                    continue;
                }
            }
        }
        else
        {
            pcb->io_just_started = 0;
        }

        prev = curr;
        curr = next;
    }
}

void startScheduler(PCB *hashmap[], Queue *readyQueue, Queue *waitingQueue, Queue *terminatedQueue, KillEvent **killHead)
{
    int clock = 0;
    int running_pid = -1;

    while (!isQueueEmpty(readyQueue) || !isQueueEmpty(waitingQueue) || running_pid != -1)
    {
        processKillEvents(killHead, hashmap, clock);

        if (running_pid != -1)
        {
            PCB *runningPCB = getPCB(hashmap, running_pid);
            if (runningPCB->killed)
            {
                enqueue(terminatedQueue, running_pid);
                running_pid = -1;
            }
        }

        while (running_pid == -1 && !isQueueEmpty(readyQueue))
        {
            int pid = dequeue(readyQueue);
            PCB *pcb = getPCB(hashmap, pid);
            if (pcb == NULL)
                continue;
            if (pcb->killed)
            {
                enqueue(terminatedQueue, pid);
                continue;
            }
            running_pid = pid;
            pcb->state = RUNNING;
            break;
        }

        if (running_pid != -1)
        {
            PCB *pcb = getPCB(hashmap, running_pid);
            sleep(1);
            pcb->execution_time++;
            pcb->remaining_burst--;

            if (pcb->io_start_time > 0 && pcb->execution_time == pcb->io_start_time && pcb->io_duration > 0 && pcb->remaining_burst > 0)
            {
                pcb->state = WAITING;
                pcb->remaining_io = pcb->io_duration;
                pcb->io_just_started = 1;
                enqueue(waitingQueue, running_pid);
                running_pid = -1;
            }
            else if (pcb->remaining_burst == 0)
            {
                pcb->state = TERMINATED;
                pcb->completion_time = clock + 1;
                enqueue(terminatedQueue, running_pid);
                running_pid = -1;
            }
        }

        clock++;
        updateWaitingProcesses(waitingQueue, readyQueue, hashmap);
    }
}

void calculateMetrics(Queue *terminatedQueue, PCB *hashmap[])
{
    Node *curr = terminatedQueue->front;
    while (curr != NULL)
    {
        PCB *pcb = getPCB(hashmap, curr->pid);
        pcb->turnaround_time = pcb->completion_time;
        if (pcb->killed)
        {
            pcb->waiting_time = pcb->burst_time - pcb->execution_time;
        }
        else
        {
            pcb->waiting_time = pcb->turnaround_time - pcb->burst_time;
        }
        curr = curr->next;
    }
}

void displayResults(Queue *terminatedQueue, PCB *hashmap[])
{
    printf("\n%-8s %-15s %-8s %-8s %-18s %-12s %-12s\n", "PID", "Name", "CPU", "IO", "Status", "Turnaround", "Waiting");

    Node *curr = terminatedQueue->front;
    while (curr != NULL)
    {
        PCB *pcb = getPCB(hashmap, curr->pid);
        if (pcb->killed)
        {
            char status[30];
            sprintf(status, "KILLED at %d", pcb->completion_time);
            printf("%-8d %-15s %-8d %-8d %-18s %-12s %-12s\n",
                   pcb->pid, pcb->name, pcb->burst_time, pcb->io_duration,
                   status, "-", "-");
        }
        else
        {
            printf("%-8d %-15s %-8d %-8d %-18s %-12d %-12d\n",
                   pcb->pid, pcb->name, pcb->burst_time, pcb->io_duration,
                   "OK", pcb->turnaround_time, pcb->waiting_time);
        }
        curr = curr->next;
    }
}

int readInput(PCB *hashmap[], Queue *readyQueue, KillEvent **killHead)
{
    printf("Enter process details (format: <name> <pid> <burst> <io_start> <io_duration>)\n");
    printf("Enter 'KILL <pid> <time>' for kill events\n");
    printf("Enter number of lines: ");

    int number = getValidInteger();
    printf("\n");
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
    char line[MAX_LINE];

    while (number--)
    {
        fgets(line, MAX_LINE, stdin);

        char command[10];
        if (sscanf(line, "%s", command) != 1)
        {
            printf("Invalid input format!\n");
            return 0;
        }

        if (strcmp(command, "KILL") == 0)
        {
            int pid, time;
            if (sscanf(line, "KILL %d %d", &pid, &time) != 2)
            {
                printf("Invalid KILL format! Use: KILL <pid> <time>\n");
                return 0;
            }

            if (!addKillEvent(killHead, pid, time))
            {
                printf("Failed to add kill event. Memory allocation failed.\n");
                return 0;
            }
        }
        else
        {
            char name[MAX_NAME];
            int pid, burst;
            char io_start[10], io_duration[10];

            int count = sscanf(line, "%s %d %d %s %s", name, &pid, &burst, io_start, io_duration);

            if (count < 3)
            {
                printf("Invalid process format!\n");
                return 0;
            }

            int io_Start = 0, io_Duration = 0;

            if (count == 5)
            {
                if (strcmp(io_start, "-") != 0)
                {
                    io_Start = atoi(io_start);
                    io_Duration = atoi(io_duration);
                }
            }

            if (!initializePCB(hashmap, readyQueue, pid, name, burst, io_Start, io_Duration))
            {
                printf("Failed to initialize process %d. Memory allocation failed.\n", pid);
                return 0;
            }
        }
    }

    return 1;
}

int main()
{
    PCB *pcbHashmap[MAX_PROCESSES] = {0};

    Queue *readyQueue = createQueue();
    Queue *waitingQueue = createQueue();
    Queue *terminatedQueue = createQueue();

    if (readyQueue == NULL || waitingQueue == NULL || terminatedQueue == NULL)
    {
        printf("Failed to create queues. Exiting...\n");
        cleanupAll(pcbHashmap, readyQueue, waitingQueue, terminatedQueue, NULL);
        return 1;
    }

    KillEvent *killHead = NULL;

    if (!readInput(pcbHashmap, readyQueue, &killHead))
    {
        cleanupAll(pcbHashmap, readyQueue, waitingQueue, terminatedQueue, &killHead);
        return 1;
    }

    startScheduler(pcbHashmap, readyQueue, waitingQueue, terminatedQueue, &killHead);
    calculateMetrics(terminatedQueue, pcbHashmap);
    displayResults(terminatedQueue, pcbHashmap);

    cleanupAll(pcbHashmap, readyQueue, waitingQueue, terminatedQueue, &killHead);

    return 0;
}