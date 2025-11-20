#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define MAX_CAPACITY 1000
#define MIN_CAPACITY 1

typedef struct Node{
    int key;
    char *value;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct HashNode{
    int key;
    Node * queueNode;
    struct HashNode *next; 
} HashNode;

typedef struct LRUCache{
    unsigned short capacity;
    unsigned short size;
    unsigned short hashMapSize;
    Node *head;
    Node *tail;
    HashNode **hashMap;
} LRUCache;

int hashFunction(int key, int hashMapSize){
    return abs(key) % hashMapSize;
}

int isPrime(int num){
    if(num <= 1) return 0;
    if(num <= 3) return 1;
    if(num % 2 == 0 || num % 3 == 0) return 0;

    for(int i = 5; i * i <= num; i += 6){
        if(num % i == 0 || num % (i + 2) == 0)
            return 0;
    }
    return 1;
}

int nextPrime(int num){
    while(!isPrime(num)){
        num++;
    }
    return num;
}

Node *createNode(int key, char *value){
    Node *newNode = (Node *)malloc(sizeof(Node));
    if(newNode == NULL){
        printf("Memeory Allocation Failed.\n");
        return NULL;
    }
    newNode -> key = key;
    newNode -> value = (char *)malloc(strlen(value) + 1);
    if(!newNode->value){
        free(newNode);
        return NULL;
    }
    strcpy(newNode -> value, value);
    newNode -> next = NULL;
    newNode -> prev = NULL;
    return newNode;
}

LRUCache *createCache(int capacity){
    LRUCache *cache = (LRUCache *)malloc(sizeof(LRUCache));
    if(cache == NULL){
        printf("Memory allocation failes.\n");
        return NULL;
    }
    cache -> capacity = capacity;
    cache -> head = NULL;
    cache -> tail = NULL;
    cache -> size = 0;
    cache -> hashMapSize = nextPrime(capacity * 2);
    cache -> hashMap = (HashNode **)calloc(cache->hashMapSize, sizeof(HashNode *));
    if(cache -> hashMap == NULL){
        printf("Memory Allocation Failed.\n");
        return NULL;
    }
    return cache;
}

HashNode *createHashNode(int key, Node *node){
    HashNode *newHashNode = (HashNode *)malloc(sizeof(HashNode));
    if(newHashNode == NULL){
        printf("Memory allocation failes.\n");
        return NULL;
    }
    newHashNode -> key = key;
    newHashNode -> queueNode = node;
    newHashNode -> next = NULL;
    return newHashNode;
}

Node *hashMapSearch(LRUCache *cache, int key){
    int index = hashFunction(key, cache -> hashMapSize);
    HashNode *current = cache -> hashMap[index];
    while(current != NULL){
        if(current -> key == key){
            return current -> queueNode;
        }
        current = current -> next;
    }
    return NULL;
}

void hashMapDelete(LRUCache *cache, int key){
    int index = hashFunction(key, cache -> hashMapSize);
    HashNode *current = cache -> hashMap[index];
    HashNode *prev = NULL;
    
    while(current != NULL){
        if(current -> key == key){
            if(prev != NULL){
                prev -> next = current -> next;
            } else {
                cache -> hashMap[index] = current -> next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current -> next;
    }
}

void removeNode(LRUCache *cache, Node *node){
    if(node -> prev){
        node -> prev -> next = node -> next;
    } else {
        cache -> head = node -> next;
    }
    
    if(node -> next){
        node -> next -> prev = node -> prev;
    } else {
        cache -> tail = node -> prev;
    }
}

void addToFront(LRUCache *cache, Node *node){
    node -> next = cache -> head;
    node -> prev = NULL;
    
    if(cache -> head){
        cache -> head -> prev = node;
    }
    cache -> head = node;
    
    if(cache -> tail == NULL){
        cache -> tail = node;
    }
}

void hashMapInsert(LRUCache *cache, int key, Node *node){
    int index = hashFunction(key, cache -> hashMapSize);
    HashNode *newHashNode = createHashNode(key, node);
    if (newHashNode == NULL) { 
        printf("Memory Allocation Failed.\n");
        return;
    }
    
    newHashNode -> next = cache -> hashMap[index];
    cache -> hashMap[index] = newHashNode;
}

void moveToFront(LRUCache *cache, Node *node){
    if(cache -> head == node){
        return;
    }
    
    removeNode(cache, node);
    addToFront(cache, node);
}

void put(LRUCache *cache, int key, char *value){
    if(cache == NULL){
        printf("Cache not initialized. Use createCache <size>\n");
        return;
    }
    
    Node *exisitingNode = hashMapSearch(cache, key);
    
    if(exisitingNode){
        free(exisitingNode -> value);
        exisitingNode -> value = (char *)malloc(strlen(value) + 1);
        strcpy(exisitingNode -> value, value);
        moveToFront(cache, exisitingNode);
        printf("Updated key %d with value: %s\n", key, value);
    } else {
        Node *newNode = createNode(key, value);
        
        if(cache -> size >= cache -> capacity){
            Node *LRU = cache -> tail;
            
            printf("Cache is Full. Evicting LRU key: %d Value: %s\n", LRU -> key, LRU -> value);

            hashMapDelete(cache, LRU->key);
            removeNode(cache , LRU);
            free(LRU -> value);
            free(LRU);
            cache -> size--;
        }
        
        addToFront(cache, newNode);
        hashMapInsert(cache, key, newNode);
        cache -> size++;
        
        printf("Inserted %d : %s\n", key, value);
    }
}

char *get(LRUCache *cache, int key){
    if(cache == NULL){
        printf("Cache is not initialize. Use createCache <size>\n");
        return NULL;
    }
    
    Node *node = hashMapSearch(cache, key);
    if(node == NULL){
        return NULL;
    }

    moveToFront(cache, node);
    return node -> value;
}

void freeCache(LRUCache *cache){
    if(cache == NULL){
        return;
    }

    Node *current = cache -> head;
    while(current){
        Node *temp = current;
        current = current -> next;
        free(temp -> value);
        free(temp);
    }
    for(int i = 0; i < cache -> hashMapSize; i++){
        HashNode *current = cache -> hashMap[i];
        while(current){
            HashNode *temp = current;
            current = current -> next;
            free(temp);
        }
    }
    free(cache -> hashMap);
    free(cache);
    printf("Freeing all Memory....\n");
}

void executeCommand(LRUCache **cache, char *buffer) {
    char command[100];
    char value[1000];
    int key;
    unsigned short capacity;
    
    if (sscanf(buffer, "%s", command) != 1) {
        printf("Invalid command format");
        return;
    }
    
    if (strcmp(command, "createCache") == 0) {
        int count = sscanf(buffer, "%*s %hu", &capacity);

        if(count != 1){
            printf("Error: Invalid input. Usage: createCache <size>\n");
            return;
        }
        
        if(capacity < MIN_CAPACITY || capacity > MAX_CAPACITY){
            printf("Size must be between 1 and 1000.\n");
            return;
        }
        
        if (*cache != NULL) {
            printf("Cache already exists.\n");
            return;
        }
        if(*cache == NULL){
            *cache = createCache(capacity);
        }
        
    } else if (strcmp(command, "put") == 0) {
        int count = sscanf(buffer, "%*s %d %s", &key, value);
        if (count != 2) {
            printf("Invalid input. Usage: put <key> <value>\n");
            return;
        }
        put(*cache, key, value);
        
    } else if (strcmp(command, "get") == 0) {
        int count = sscanf(buffer, "%*s %d", &key);
            if(count != 1){
                printf("Error: Invalid input. Usage: get <key>\n");
                return;
            }
        
        char *result = get(*cache, key);
        if (result) {
            printf("%s\n", result);
        } else {
            printf("NULL\n");
        }
        
    } else if (strcmp(command, "exit") == 0) {
        if (*cache != NULL) {
            freeCache(*cache);
            *cache = NULL;
        }
        exit(0);
        
    } else {
        printf("Enter correct command!\n");
    }
}

int main(){
    LRUCache *cache = NULL;

    char buffer[100];
    printf("========Input Command Format========\n");
    printf("1.createCache <size>\n");
    printf("2.put <key> <value>\n");
    printf("3.get <key>\n");
    printf("4.exit\n\n");
    printf("Enter Command\n");

    while (1) {
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            break;
        }
        
        buffer[strcspn(buffer, "\n")] = 0;
        
        executeCommand(&cache, buffer);
    }

    if (cache != NULL) {
        freeCache(cache);
    }

    return 0;
}