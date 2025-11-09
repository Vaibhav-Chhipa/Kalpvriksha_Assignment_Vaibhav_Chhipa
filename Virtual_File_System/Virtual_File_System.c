#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>

#define BLOCK_SIZE 512
#define MAX_BLOCKS 100
#define MAX_NAME_LENGTH 50

typedef struct FreeBlock {
    unsigned short index;
    struct FreeBlock *next;
    struct FreeBlock *prev;
} FreeBlock;

typedef struct FileNode {
    char name[MAX_NAME_LENGTH + 1];
    bool isDirectory;
    struct FileNode *next;
    struct FileNode *child;
    struct FileNode *parent;
    unsigned short *blockPointers;
    unsigned short blocksAllocated;
    unsigned int contentSize;
} FileNode;

typedef struct VFS{
    char virtualDisk[MAX_BLOCKS][BLOCK_SIZE];
    FreeBlock *freeListHead;
    FreeBlock *freeListTail;
    unsigned short totalUsed;
} VFS;

char *readInputLine(){
    int character;
    size_t size = 100;
    size_t totalLength = 0;
    char *buffer = malloc(size);

    if(!buffer){
        printf("Memory allocation failed.\n");
        return NULL;
    }

    while ((character = getchar()) != '\n' && character != EOF){
        buffer[totalLength++] = (char)character;
        if(totalLength + 1 >= size){
            size = size * 2;
            char *newBuffer = realloc(buffer, size);
            if(!newBuffer){
                free(buffer);
                printf("Memory allocation failed.\n");
                return NULL;
            }
            buffer = newBuffer;
        }
    }

    if(character == EOF && totalLength == 0){
        free(buffer);
        return NULL;
    }

    buffer[totalLength] = '\0';
    return buffer;
}

void initializeFreeBlocks(VFS *vfs){
    for(unsigned short i = 0; i < MAX_BLOCKS; i++){
        FreeBlock *newBlock = malloc(sizeof(FreeBlock));
        
        if (!newBlock) {
            printf("Memory allocation failed for free block.\n");

            // Free all previously allocated blocks
            FreeBlock *temp = vfs->freeListHead;
            while(temp != NULL){
                FreeBlock *next = temp->next;
                free(temp);
                temp = next;
            }

            vfs->freeListHead = vfs->freeListTail = NULL;
            return;
        }

        newBlock -> index = i;
        newBlock -> next = NULL;
        newBlock -> prev = vfs -> freeListTail;

        if(vfs -> freeListTail != NULL){
            vfs -> freeListTail -> next = newBlock;
        } else {
            vfs -> freeListHead = newBlock;
        }
        vfs -> freeListTail = newBlock;
    }
}

FileNode *getFileNodeByName(FileNode *cwd, const char *name){
    if(cwd->child == NULL) {
        return NULL;
    }
    FileNode *temp = cwd->child;
    do {
        if(strcmp(temp->name, name) == 0) {
            return temp;
        }
        temp = temp->next;
    } while(temp != cwd->child);
    
    return NULL;
}

FileNode *initializeDirectory(const char *dirName, FileNode *parent){
    FileNode *newDirectory = calloc(1, sizeof(FileNode));
    if (!newDirectory) {
        printf("Memory allocation failed for directory.\n");
        return NULL;
    }

    strncpy(newDirectory->name, dirName, MAX_NAME_LENGTH);
    newDirectory->name[MAX_NAME_LENGTH] = '\0';
    newDirectory->isDirectory = true;
    newDirectory->child = NULL;
    newDirectory->parent = parent;
    newDirectory->contentSize = 0;
    newDirectory->blockPointers = NULL;
    newDirectory->blocksAllocated = 0;
    newDirectory->next = newDirectory;
    
    return newDirectory;
}

FileNode *initializeFile(const char *fileName, FileNode *parent){
    FileNode *newFile = calloc(1, sizeof(FileNode));
    if (!newFile) {
        printf("Memory allocation failed for file.\n");
        return NULL;
    }
    
    strncpy(newFile->name, fileName, MAX_NAME_LENGTH);
    newFile->name[MAX_NAME_LENGTH] = '\0';
    newFile->isDirectory = false;
    newFile->parent = parent;
    newFile->contentSize = 0;
    newFile->blockPointers = NULL;
    newFile->blocksAllocated = 0;
    newFile->next = newFile;
    
    return newFile;
}

void insertFileNode(FileNode *cwd, FileNode *newNode) {
    if(cwd->child == NULL) {
        cwd->child = newNode;
        newNode->next = newNode;
    } else {
        FileNode *temp = cwd->child;
        while(temp->next != cwd->child) {
            temp = temp->next;
        }
        temp->next = newNode;
        newNode->next = cwd->child;
    }
}

void createDirectory(FileNode *cwd, const char *dirName){
    if(getFileNodeByName(cwd, dirName) != NULL) {
        printf("Name already exists in current directory.\n");
        return;
    }

    FileNode *newDirectory = initializeDirectory(dirName, cwd);
    if (!newDirectory) {
        printf("Memory allocation failed while creating directory.\n");
        return;
    }

    insertFileNode(cwd, newDirectory);
    printf("Directory '%s' created successfully\n", dirName);
}

void createFile(FileNode *cwd, const char *fileName){
    if(getFileNodeByName(cwd, fileName) != NULL) {
        printf("Name already exists in current directory.\n");
        return;
    }

    FileNode *newFile = initializeFile(fileName, cwd);
    if (!newFile) {
        return;
    }

    insertFileNode(cwd, newFile);
    printf("File '%s' created successfully.\n", fileName);
}

void processEscapeSequences(const char *input, char *output) {
    unsigned short i = 0, j = 0;
    while (input[i] != '\0') {
        if (input[i] == '\\' && input[i + 1] != '\0') {
            switch (input[i + 1]) {
                case 'n':
                    output[j++] = '\n';
                    i += 2;
                    break;
                case 't':
                    output[j++] = '\t';
                    i += 2;
                    break;
                case '\\':
                    output[j++] = '\\';
                    i += 2;
                    break;
                default:
                    output[j++] = input[i++];
                    break;
            }
        } else {
            output[j++] = input[i++];
        }
    }
    output[j] = '\0';
}

void writeInFile(VFS *vfs, FileNode *cwd, const char *filename, const char *text) {
    FileNode *file = getFileNodeByName(cwd, filename);
    if (file == NULL) {
        printf("File not found.\n");
        return;
    }

    if (file->isDirectory) {
        printf("File not found.\n");
        return;
    }

    char *processedText = malloc(strlen(text) * 2 + 1);
    if (!processedText) {
        printf("Memory allocation failed.\n");
        return;
    }
    processEscapeSequences(text, processedText);

    unsigned int totalBytes = strlen(processedText);
    unsigned int bytesRemaining = totalBytes;
    unsigned int currentBlockIndex = file->contentSize / BLOCK_SIZE;
    unsigned int offsetInBlock = file->contentSize % BLOCK_SIZE;
    unsigned int textIndex = 0;

    while (bytesRemaining > 0) {
        // Check if we need to allocate a new block
        if (currentBlockIndex >= file->blocksAllocated) {
            // Reallocate blockPointers array
            unsigned short *newPointers = realloc(file->blockPointers, (file->blocksAllocated + 1) * sizeof(unsigned short));
            if (!newPointers) {
                printf("Memory allocation failed.\n");
                free(processedText);
                return;
            }
            file->blockPointers = newPointers;
            
            // Allocate new disk block
            if (vfs->freeListHead == NULL) {
                printf("Disk full. Memory allocation failed.\n");
                free(processedText);
                return;
            }

            FreeBlock *block = vfs->freeListHead;
            vfs->freeListHead = vfs->freeListHead->next;
            if (vfs->freeListHead != NULL) {
                vfs->freeListHead->prev = NULL;
            } else {
                vfs->freeListTail = NULL;
            }

            file->blockPointers[file->blocksAllocated] = block->index;
            file->blocksAllocated++;
            free(block);
            vfs->totalUsed++;
        }

        unsigned short blockIndex = file->blockPointers[currentBlockIndex];
        unsigned int positionInBlock = offsetInBlock;

        while (positionInBlock < BLOCK_SIZE && bytesRemaining > 0) {
            vfs->virtualDisk[blockIndex][positionInBlock++] = processedText[textIndex++];
            bytesRemaining--;
        }

        currentBlockIndex++;
        offsetInBlock = 0;
    }

    file->contentSize += totalBytes;
    printf("Data written successfully (size=%u bytes).\n", totalBytes);
    free(processedText);
}


void readInFile(const VFS *vfs, FileNode *cwd, const char *fileName){
    FileNode *file = getFileNodeByName(cwd, fileName);
    if (file == NULL) {
        printf("File not found.\n");
        return;
    }
    
    if (file->isDirectory) {
        printf("File not found.\n");
        return;
    }
    unsigned int size = file->contentSize;
    if(size == 0){
        printf("(empty)\n");
        return;
    }
    
    unsigned int printed = 0;
    for (unsigned short b = 0; b < file->blocksAllocated && printed < size; b++) {
        unsigned short blockIndex = file->blockPointers[b];
        for (unsigned short j = 0; j < BLOCK_SIZE && printed < size; j++, printed++) {
            printf("%c", vfs->virtualDisk[blockIndex][j]);
        }
    }
    printf("\n");
}

void freeFileBlocks(VFS *vfs, FileNode *file) {
    if (file->blockPointers == NULL || file->isDirectory) {
        return;
    }
    
    for(unsigned short b = 0; b < file->blocksAllocated; b++){
        FreeBlock *freeBlock = malloc(sizeof(FreeBlock));
        if (!freeBlock) {
            printf("Memory allocation failed during cleanup.\n");
            continue;
        }
        freeBlock->index = file->blockPointers[b];
        freeBlock->next = NULL;
        freeBlock->prev = vfs->freeListTail;
        if(vfs->freeListTail){
            vfs->freeListTail->next = freeBlock;
        } else {
            vfs->freeListHead = freeBlock;
        }
        vfs->freeListTail = freeBlock;
        
        if (vfs->totalUsed > 0){
            vfs->totalUsed--;
        } 
    }
    
    free(file->blockPointers);
    file->blockPointers = NULL;
    file->blocksAllocated = 0;
}

void removeFileNode(FileNode *cwd, FileNode *node) {
    if(node->next == node){
        cwd->child = NULL;
    } else {
        FileNode *prevNode = cwd->child;
        while(prevNode->next != node){
            prevNode = prevNode->next;
        }
        prevNode->next = node->next;
        
        if(cwd->child == node){
            cwd->child = node->next;
        }
    }
}

void deleteFile(VFS *vfs, FileNode *cwd, const char *fileName){
    FileNode *file = getFileNodeByName(cwd, fileName);
    if (file == NULL) {
        printf("File not found.\n");
        return;
    }
    
    if (file->isDirectory) {
        printf("File not found.\n");
        return;
    }
    
    freeFileBlocks(vfs, file);
    removeFileNode(cwd, file);
    free(file);
    printf("File '%s' deleted successfully.\n", fileName);
}

void removeDirectory(VFS *vfs, FileNode *cwd, char *dirName){
    FileNode *dir = getFileNodeByName(cwd, dirName);
    if (dir == NULL) {
        printf("Directory not found.\n");
        return;
    }
    
    if (!dir->isDirectory) {
        printf("Directory not found.\n");
        return;
    }

    if(dir->child != NULL){
        printf("Directory not empty. Remove files first.\n");
        return;
    }
    
    removeFileNode(cwd, dir);
    free(dir);
    printf("Directory removed successfully.\n");
}

void listAllEntries(const FileNode *cwd){
    if(cwd -> child == NULL){
        printf("(empty)\n");
        return;
    }

    FileNode *temp = cwd -> child;

    do {
        if(temp -> isDirectory){
            printf("%s/\n", temp -> name);
        } else {
            printf("%s\n", temp -> name);
        }
        temp = temp -> next;
    } while(temp != cwd -> child);
}

void printWorkingDirectoryRecursive(FileNode *node) {
    if (node->parent != node) {
        printWorkingDirectoryRecursive(node->parent);
        printf("/%s", node->name);
    }
}

void printWorkingDirectory(FileNode *cwd){
    if (cwd->parent == cwd) {
        printf("/\n");
    } else {
        printWorkingDirectoryRecursive(cwd);
        printf("\n");
    }
}

FileNode *changeDirectory(FileNode *cwd, const char * directoryName){
    if(strcmp(directoryName, "..") == 0){
        if(cwd -> parent != cwd){
            cwd = cwd->parent;
            printf("Moved to ");    
            printWorkingDirectory(cwd);
        }
        return cwd;
    }

    FileNode *dir = getFileNodeByName(cwd, directoryName);
    if(dir == NULL){
        printf("Directory not found.\n");
        return cwd;
    }

    if (!dir->isDirectory) {
        printf("Directory not found.\n");
        return cwd;
    }

    printf("Moved to ");
    printWorkingDirectory(dir);
    return dir;
}

void displayDiskUsage(const VFS *vfs){
    printf("Total Blocks: %d\n", MAX_BLOCKS);
    printf("Used Blocks: %hu\n", vfs->totalUsed);
    printf("Free Blocks: %hu\n", (unsigned short)MAX_BLOCKS - vfs->totalUsed);
    printf("Disk Usage: %.2f%%\n", (float) (vfs->totalUsed * 100) / MAX_BLOCKS);
}

void freeDirectoryTree(VFS *vfs, FileNode *node){
    if(node == NULL) return;
    
    if(node->child != NULL){
        FileNode *temp = node->child;
        FileNode *start = temp;
        do {
            FileNode *next = temp->next;
            freeDirectoryTree(vfs, temp);
            temp = next;
        } while(temp != start);
    }
    
    if(!node->isDirectory){
        freeFileBlocks(vfs, node);
    }
    
    free(node);
}

void freeAllMemory(VFS *vfs, FileNode *root){
    if(root != NULL) {
        freeDirectoryTree(vfs, root);
    }

    FreeBlock *temp = vfs->freeListHead;
    while(temp != NULL){
        FreeBlock *next = temp->next;
        free(temp);
        temp = next;
    }
}

void cleanup(VFS *vfs, FileNode *cwd) {
    FileNode *root = cwd;
    while (root->parent != root) {
        root = root->parent;
    }
    freeAllMemory(vfs, root);
    free(vfs);
}

bool handleCommand(VFS *vfs, FileNode **cwd, char *line) {
    char command[20];
    if (sscanf(line, "%19s", command) != 1) return true;

    if (strcmp(command, "mkdir") == 0) {
        char dirName[MAX_NAME_LENGTH + 1];
        if (sscanf(line, "%*s %50s", dirName) == 1) {
            createDirectory(*cwd, dirName);
        } else {
            printf("Usage: mkdir <directory_name>\n");
        }

    } else if (strcmp(command, "create") == 0) {
        char fileName[MAX_NAME_LENGTH + 1];
        if (sscanf(line, "%*s %50s", fileName) == 1) {
            createFile(*cwd, fileName);
        } else {
            printf("Usage: create <file_name>\n");
        }

    } else if (strcmp(command, "write") == 0) {
        char filename[MAX_NAME_LENGTH + 1];
        char *firstQuote = strchr(line, '"');
        char *lastQuote = strrchr(line, '"');

        if (!firstQuote || !lastQuote || firstQuote == lastQuote) {
            printf("Usage: write <filename> \"<text>\"\n");
        } else {
            if (sscanf(line, "%*s %50s", filename) != 1) {
                printf("Usage: write <filename> \"<text>\"\n");
            } else {
                int len = lastQuote - firstQuote - 1;
                char *content = malloc(len + 1);
                if (!content) {
                    printf("Memory allocation failed.\n");
                    return true;
                }
                strncpy(content, firstQuote + 1, len);
                content[len] = '\0';
                writeInFile(vfs, *cwd, filename, content);
                free(content);
            }
        }

    } else if (strcmp(command, "read") == 0) {
        char filename[MAX_NAME_LENGTH + 1];
        if (sscanf(line, "%*s %50s", filename) == 1) {
            readInFile(vfs, *cwd, filename);
        } else {
            printf("Usage: read <filename>\n");
        }

    } else if (strcmp(command, "delete") == 0) {
        char filename[MAX_NAME_LENGTH + 1];
        if (sscanf(line, "%*s %50s", filename) == 1) {
            deleteFile(vfs, *cwd, filename);
        } else {
            printf("Usage: delete <filename>\n");
        }

    } else if (strcmp(command, "rmdir") == 0) {
        char dirName[MAX_NAME_LENGTH + 1];
        if (sscanf(line, "%*s %50s", dirName) == 1) {
            removeDirectory(vfs, *cwd, dirName);
        } else {
            printf("Usage: rmdir <directory_name>\n");
        }

    } else if (strcmp(command, "ls") == 0) {
        listAllEntries(*cwd);

    } else if (strcmp(command, "cd") == 0) {
        char dirName[MAX_NAME_LENGTH + 1];
        if (sscanf(line, "%*s %50s", dirName) == 1) {
            *cwd = changeDirectory(*cwd, dirName);
        } else {
            printf("Usage: cd <directory_name>\n");
        }

    } else if (strcmp(command, "pwd") == 0) {
        printWorkingDirectory(*cwd);

    } else if (strcmp(command, "df") == 0) {
        displayDiskUsage(vfs);

    } else if (strcmp(command, "exit") == 0) {
        printf("Memory released. Exiting program...\n");
        // Navigate to root before freeing
        cleanup(vfs, *cwd);
        *cwd = NULL;
        return false;
    } else {
        printf("Enter correct command!\n");
    }
    return true;
}


int main(){
    //Initialization
    VFS *vfs = malloc(sizeof(VFS));
    if (!vfs) {
        printf("Memory allocation failed for VFS.\n");
        return 1;
    }
    vfs->freeListHead = NULL;
    vfs->freeListTail = NULL;
    vfs->totalUsed = 0;
    
    initializeFreeBlocks(vfs);
    if (vfs->freeListHead == NULL) {
        printf("Failed to initialize free blocks.\n");
        free(vfs);
        return 1;
    }

    FileNode *root = initializeDirectory("/", NULL);
    if (!root) {
        freeAllMemory(vfs, NULL);
        free(vfs);
        return 1;
    }
    root->parent = root;

    FileNode *cwd = root;

    printf("Free block list initialized successfully.\n");
    printf("Compact VFS - ready. Type 'exit' to quit.\n");
    
    while(1){
        printf("\n%s> ", cwd->name);
        
        char *line = readInputLine();
        if(!line){
            printf("\nMemory released. Exiting program...\n");
            cleanup(vfs, cwd);
            break;
        }
        bool keepRunning = handleCommand(vfs, &cwd, line);
        free(line);
        if(!keepRunning){
            break;
        }
    }
    return 0;
}