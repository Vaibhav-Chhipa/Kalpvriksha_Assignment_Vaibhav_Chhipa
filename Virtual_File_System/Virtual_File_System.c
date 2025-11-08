#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <stdbool.h>

#define BLOCK_SIZE 512
#define MAX_BLOCKS 1024
#define BLOCKS_PER_FILE 1024
#define MAX_NAME_LENGTH 51
#define MAX_CONTENT 1000
#define MAX_LINE_LENGTH 1024

typedef struct FreeBlock {
    unsigned short index;
    struct FreeBlock *next;
    struct FreeBlock *prev;
} FreeBlock;

typedef struct FileNode {
    char name[MAX_NAME_LENGTH];
    bool isDirectory;
    struct FileNode *next;
    struct FileNode *child;
    struct FileNode *parent;
    short blockPointers[BLOCKS_PER_FILE];
    unsigned int contentSize;
} FileNode;

typedef struct VFS{
    char virtualDisk[MAX_BLOCKS][BLOCK_SIZE];
    FreeBlock *freeListHead;
    FreeBlock *freeListTail;
    unsigned short totalUsed;
} VFS;

void initBlockPointers(FileNode *file) {
    for (short i = 0; i < BLOCKS_PER_FILE; i++){
        file->blockPointers[i] = -1;
    }
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
            exit(1);
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

void createDirectory(FileNode *cwd, const char *dirName){
    FileNode *temp = cwd->child;
    if(temp != NULL){
        do{
            if(strcmp(temp->name, dirName) == 0){
                printf("Name already exists in current directory.\n");
                return;
            }
            temp = temp -> next;    
        } while (temp != cwd->child);
    }

    FileNode *newDirectory = calloc(1 ,sizeof(FileNode));
    if (!newDirectory) {
        printf("Memory allocation failed for directory.\n");
        exit(1);
    }
    strcpy(newDirectory -> name, dirName);
    newDirectory -> isDirectory = true;
    newDirectory -> child = NULL;
    newDirectory -> parent = cwd;
    newDirectory -> contentSize = 0;
    initBlockPointers(newDirectory);

    if(cwd -> child == NULL){
        cwd -> child = newDirectory;
        newDirectory -> next = newDirectory;
    } else {
        FileNode *temp = cwd->child;
        while(temp -> next != cwd -> child){
            temp = temp -> next;
        }
        temp -> next = newDirectory;
        newDirectory -> next = cwd -> child;
    }
    printf("Directory '%s' created successfully\n", dirName);
}

void createFile(FileNode *cwd, const char *fileName){
    FileNode *temp = cwd -> child;
    if(temp != NULL){
        do{
            if(strcmp(temp -> name, fileName) == 0){
                printf("Name already exists in current directory.\n");
                return;
            }
            temp = temp->next;
        } while(temp != cwd -> child);
    }

    FileNode *newFile = calloc(1, sizeof(FileNode));
    if (!newFile) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    strcpy(newFile->name, fileName);
    newFile->isDirectory = false;
    newFile -> parent = cwd;
    newFile -> contentSize = 0;
    initBlockPointers(newFile);

    if (cwd->child == NULL) {
        cwd->child = newFile;
        newFile->next = newFile;  
    } else {
        FileNode *lastNode = cwd->child;
        while (lastNode->next != cwd->child) {
            lastNode = lastNode->next;
        }
        lastNode->next = newFile;
        newFile->next = cwd->child;
    }
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
    if (cwd->child == NULL) {
        printf("(empty)\n");
        return;
    }

    FileNode *temp = cwd->child;

    char processedText[MAX_CONTENT];
    processEscapeSequences(text, processedText);

    do {
        if (strcmp(temp->name, filename) == 0 && !temp->isDirectory) {
            unsigned int totalBytes = strlen(processedText);
            unsigned int bytesRemaining = totalBytes;

            // Start writing from the end of the file
            unsigned int currentBlockIndex  = temp->contentSize / BLOCK_SIZE;
            unsigned int offsetInBlock = temp->contentSize % BLOCK_SIZE;
            unsigned int textIndex = 0;

            while (bytesRemaining > 0) {
                if (currentBlockIndex  >= BLOCKS_PER_FILE) {
                    printf("Error: File size exceeds maximum limit (%d bytes)\n", BLOCKS_PER_FILE * BLOCK_SIZE);
                    return;
                }

                unsigned int blockIndex;
                // Allocate new block if needed
                if (temp->blockPointers[currentBlockIndex ] == -1) {
                    if (vfs->freeListHead == NULL) {
                        printf("Disk Full\n");
                        return;
                    }

                    FreeBlock *block = vfs->freeListHead;
                    vfs->freeListHead = vfs->freeListHead->next;
                    if (vfs->freeListHead != NULL)
                        vfs->freeListHead->prev = NULL;

                    blockIndex = block->index;
                    free(block);

                    temp->blockPointers[currentBlockIndex ] = blockIndex;
                    vfs->totalUsed++;
                }

                blockIndex = temp->blockPointers[currentBlockIndex ];
                unsigned int positionInBlock  = offsetInBlock ;

                while (positionInBlock  < BLOCK_SIZE && bytesRemaining > 0) {
                    vfs->virtualDisk[blockIndex][positionInBlock++] = processedText[textIndex++];
                    bytesRemaining--;
                }

                currentBlockIndex ++;
                offsetInBlock  = 0;
            }

            if (temp->contentSize + totalBytes > BLOCKS_PER_FILE * BLOCK_SIZE) {
                temp->contentSize = BLOCKS_PER_FILE * BLOCK_SIZE;
            } else {
                temp->contentSize += totalBytes;
            }

            printf("Data written successfully (size=%d bytes).\n", totalBytes);
            return;
        }

        temp = temp->next;
    } while (temp != cwd->child);

    printf("File not found.\n");
}


void readInFile(const VFS *vfs, const FileNode *cwd, const char *fileName){
    if(cwd -> child == NULL){
        printf("(empty)\n");
        return;
    }
    FileNode *temp = cwd -> child;
    do{
        if(strcmp(temp -> name, fileName) == 0 && !temp -> isDirectory){
            
            unsigned int size = temp->contentSize;
            unsigned int printed = 0;
            if(size == 0){
                printf("(empty)\n");
                return;
            }
            for (unsigned short b = 0; b < BLOCKS_PER_FILE && temp->blockPointers[b] != -1 && printed < size; b++) {
                unsigned short blockIndex = temp->blockPointers[b];
                for (unsigned short j = 0; j < BLOCK_SIZE && printed < size; j++, printed++) {
                    printf("%c", vfs -> virtualDisk[blockIndex][j]);
                }
            }
            printf("\n");
            return;
        }  
        temp = temp -> next;
    }while(temp != cwd -> child);
    printf("File not found.\n");
}

void freeBlockIndex(VFS *vfs , unsigned short index){
    FreeBlock *freeBlock = malloc(sizeof(FreeBlock));
    if (!freeBlock) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    freeBlock -> index = index;
    freeBlock -> next = NULL;
    freeBlock -> prev = vfs -> freeListTail;
    if(vfs -> freeListTail){
        vfs -> freeListTail -> next = freeBlock;
    } else {
        vfs -> freeListHead = freeBlock;
    }
    vfs -> freeListTail = freeBlock;
}

void deleteFile(VFS *vfs, FileNode *cwd, const char *fileName){
    if(cwd -> child == NULL){
        printf("No files in current directory.\n");
        return;
    }

    FileNode *temp = cwd -> child;
    
    do{
        if(strcmp(temp -> name, fileName) == 0 && !temp -> isDirectory){
            for(unsigned short b = 0; b < BLOCKS_PER_FILE; b++){
                if(temp -> blockPointers[b] != -1){
                    freeBlockIndex(vfs, temp -> blockPointers[b]);
                    temp -> blockPointers[b] = -1;
                    if (vfs -> totalUsed > 0) vfs -> totalUsed--;
                } else {
                    break;
                }
            }
            if(temp -> next == temp){
                cwd -> child = NULL;
            } else{
                FileNode *prevNode = temp;

                while(prevNode -> next != temp){
                    prevNode = prevNode -> next;
                }
                prevNode -> next = temp -> next;

                if(cwd -> child == temp){
                    cwd -> child = temp -> next;
                }
            } 
            free(temp);
            printf("File '%s' deleted successfully.\n", fileName);
            return;
        }
        temp = temp -> next;
    } while(temp != cwd -> child);
    printf("File not found.\n");
}

void removeDirectory(VFS *vfs, FileNode *cwd, char *dirName){
    if(cwd -> child == NULL){
        printf("Directory not found.\n");
        return;
    }

    FileNode *temp = cwd -> child;

    do {
        if(strcmp(temp->name, dirName) == 0 && temp->isDirectory){
            if(temp -> child != NULL){
                printf("Directory not empty. Remove files first.\n");
                return;
            }
            if(temp->next == temp){
                cwd->child = NULL;
            } else {
                FileNode *prevNode = temp;
                while(prevNode -> next != temp){
                    prevNode = prevNode -> next;
                }
                prevNode -> next = temp -> next;

                if(cwd -> child == temp){
                    cwd -> child = temp -> next;
                }

            }
            free(temp);
            printf("Directory removed successfully.\n");
            return;
        }
        temp = temp->next;
    } while (temp != cwd -> child);
    printf("Directory not found.\n");
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

void printWorkingDirectory(FileNode *cwd){
    char path[500] = "";
    char temp[500] = "";
    
    FileNode *current = cwd;

    while (current != NULL && current->parent != current) {
        sprintf(temp, "/%s%s", current->name, path);
        strcpy(path, temp);
        current = current->parent;
    }

    if (strlen(path) == 0) {
        printf("/\n");
    } else {
        printf("%s\n", path);
    }
}

FileNode *changeDirectory(FileNode *cwd, const char * directoryName){
    if(strcmp(directoryName, "..") == 0){
        if(cwd -> parent != NULL){
            cwd = cwd->parent;
            printf("Moved to ");    
            printWorkingDirectory(cwd);
        }
        return cwd;
    }

    FileNode *temp = cwd -> child;
    if(temp == NULL){
        printf("Directory not found.\n");
        return cwd;
    }

    do {
        if(temp -> isDirectory && strcmp(temp -> name, directoryName) == 0){
            printf("Moved to ");
            printWorkingDirectory(temp);
            return temp;
        }
        temp = temp -> next;
    } while (temp != cwd -> child);

    printf("Directory not found.\n");
    return cwd;
}

void displayDiskUsage(const VFS *vfs){
    printf("Total Blocks: %d\n", MAX_BLOCKS);
    printf("Used Blocks: %d\n", vfs->totalUsed);
    printf("Free Blocks: %d\n", MAX_BLOCKS - vfs->totalUsed);
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
        for(unsigned short i = 0; i < BLOCKS_PER_FILE; i++){
            if(node->blockPointers[i] != -1){
                freeBlockIndex(vfs, node->blockPointers[i]);
                if(vfs->totalUsed > 0) vfs->totalUsed--;
            } else {
                break;
            }
        }
    }
    
    free(node);
}

void freeAllMemory(VFS *vfs, FileNode *root){
    freeDirectoryTree(vfs, root);

    FreeBlock *temp = vfs->freeListHead;
    while(temp != NULL){
        FreeBlock *next = temp->next;
        free(temp);
        temp = next;
    }
}


int main(){
    //Initialization
    VFS *vfs = malloc(sizeof(VFS));
    if (!vfs) {
        printf("Memory allocation failed for free block.\n");
        exit(1);
    }
    vfs->freeListHead = NULL;
    vfs->freeListTail = NULL;
    vfs->totalUsed = 0;
    
    FileNode *root = NULL;
    FileNode *cwd = NULL;

    root = (FileNode *)malloc(sizeof(FileNode));
    strcpy(root->name, "/");
    root -> isDirectory = 1;
    root -> parent = root;
    root -> child = NULL;
    root -> next = root;
    initBlockPointers(root);
    cwd = root;

    initializeFreeBlocks(vfs);
    printf("Free block list initialized successfully.\n");
    printf("Compact VFS - ready. Type 'exit' to quit.\n");
    
    while(1){
        printf("\n%s> ", cwd->name);
        char line[MAX_LINE_LENGTH]; 
        if (!fgets(line, sizeof(line), stdin)) {
            printf("Input error.\n");
            continue;
        }

        line[strcspn(line, "\n")] = '\0';

        char command[20];
        if (sscanf(line, "%19s", command) != 1) continue; 

        if (strcmp(command, "mkdir") == 0) {
            char dirName[MAX_NAME_LENGTH];  
            if (sscanf(line, "%*s %50s", dirName) == 1) {
                createDirectory(cwd, dirName);
            } else {
                printf("Usage: mkdir <directory_name>\n");
            }

        } else if (strcmp(command, "create") == 0) {
            char fileName[MAX_NAME_LENGTH];
            if (sscanf(line, "%*s %50s", fileName) == 1) {
                createFile(cwd, fileName);
            } else {
                printf("Usage: create <file_name>\n");
            }

        } else if (strcmp(command, "write") == 0) {
            char filename[MAX_NAME_LENGTH];
            char content[MAX_CONTENT];

            char *firstQuote = strchr(line, '"');
            char *lastQuote  = strrchr(line, '"');

            if (!firstQuote || !lastQuote || firstQuote == lastQuote) {
                printf("Usage: write <filename> \"<text>\"\n");
            } else {
                if (sscanf(line, "%*s %50s", filename) != 1) {
                    printf("Usage: write <filename> \"<text>\"\n");
                } else {
                    int len = lastQuote - firstQuote - 1;
                    if (len >= sizeof(content)) len = sizeof(content) - 1;

                    strncpy(content, firstQuote + 1, len);
                    content[len] = '\0';

                    writeInFile(vfs, cwd, filename, content);
                }
            }

        } else if (strcmp(command, "read") == 0) {
            char filename[MAX_NAME_LENGTH];
            if (sscanf(line, "%*s %50s", filename) == 1) {
                readInFile(vfs, cwd, filename);
            } else {
                printf("Usage: read <filename>\n");
            }

        } else if (strcmp(command, "delete") == 0) {
            char filename[MAX_NAME_LENGTH];
            if (sscanf(line, "%*s %50s", filename) == 1) {
                deleteFile(vfs, cwd, filename);
            } else {
                printf("Usage: delete <filename>\n");
            }

        } else if (strcmp(command, "rmdir") == 0) {
            char dirName[MAX_NAME_LENGTH];
            if (sscanf(line, "%*s %50s", dirName) == 1) {
                removeDirectory(vfs, cwd, dirName);
            } else {
                printf("Usage: rmdir <directory_name>\n");
            }

        } else if (strcmp(command, "ls") == 0) {
            listAllEntries(cwd);

        } else if (strcmp(command, "cd") == 0) {
            char dirName[MAX_NAME_LENGTH];
            if (sscanf(line, "%*s %50s", dirName) == 1) {
                cwd = changeDirectory(cwd, dirName);
            } else {
                printf("Usage: cd <directory_name>\n");
            }

        } else if (strcmp(command, "pwd") == 0) {
            printWorkingDirectory(cwd);

        } else if (strcmp(command, "df") == 0) {
            displayDiskUsage(vfs);

        } else if (strcmp(command, "exit") == 0) {
            printf("Memory released. Exiting program...\n");
            freeAllMemory(vfs, root);
            free(vfs);
            exit(0);

        } else {
            printf("Enter correct command!\n");
        }
    }
    return 0;
}