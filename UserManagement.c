#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define FILENAME "users.txt"
#define MAX_USERS 100

// Global file pointer to keep file open in append mode
FILE *userFile = NULL; 

typedef struct {
    unsigned int id;
    char name[50];
    unsigned short age;
} User;

unsigned int currentId = 0;

int loadUsers(User users[]) {
    FILE *file = fopen(FILENAME, "r");
    if(file == NULL) return 0;
    int userCount = 0;
    while(userCount < MAX_USERS && fscanf(file, "%u,%[^,],%hu\n", &users[userCount].id, users[userCount].name, &users[userCount].age) == 3) {
        userCount++;
    }
    fclose(file);
    return userCount;
}

void saveUsers(const User users[], int userCount) {
    FILE *file = fopen(FILENAME, "w");
    if(file == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }
    for(int i = 0; i < userCount; i++) {
        fprintf(file, "%u,%s,%hu\n", users[i].id, users[i].name, users[i].age);
    }
    fclose(file);
}

void initSystem() {
    FILE *file = fopen(FILENAME, "r");
    if(file == NULL) return; 

    User u;
    while(fscanf(file, "%u,%[^,],%hu\n", &u.id, u.name, &u.age) == 3) {
        if (u.id > currentId){
            currentId = u.id;
        }
    }
    fclose(file);
}

int getIntegerInput(const char *text){
    char buffer[100];
    int value;
    char extra;
    while(1){
        printf("%s", text);
        if(!fgets(buffer, sizeof(buffer), stdin)) continue;
        buffer[strcspn(buffer, "\n")] = '\0';
        if(strlen(buffer) == 0) return -1;
        if(sscanf(buffer, "%d %c", &value, &extra) == 1) return value;
        printf("Invalid input! (Enter to cancel operation)\n");
    }
}

void getStringInput(const char *text, char *userName, size_t size){
    printf("%s", text);
    if(fgets(userName, size, stdin) != NULL){
        // remove newline if present
        userName[strcspn(userName, "\n")] = '\0'; 
    }
}

void createUser(){
    User newUser;

    if (currentId >= MAX_USERS) {
        printf("User limit reached! Cannot add more users.\n");
        return;
    }

    currentId++;
    newUser.id = currentId;

    printf("Your Unique ID is: %u\n", newUser.id);

    getStringInput("Enter Name: ", newUser.name, sizeof(newUser.name));

    int age = getIntegerInput("Enter Age: ");
    newUser.age = (unsigned short)age;
    
    if(!userFile){
        printf("File not available!\n");
        return;
    }
    fprintf(userFile, "%u,%s,%hu\n", newUser.id, newUser.name, newUser.age);
    //ensure data is written immediately
    fflush(userFile);

    printf("User Added\n");
}

void readUser(){
    FILE *file = fopen(FILENAME, "r");
    if(file == NULL){
        printf("No such file exists.\n");
        return;
    }

    User user;

    while (fscanf(file, "%u,%[^,],%hu\n", &user.id, user.name, &user.age) == 3) {
        printf("\nUser ID : %u \nUser Name : %s \nUser Age : %hu\n", user.id, user.name, user.age);
    }
    
    fclose(file);
}

void updateUser(){
    User users[MAX_USERS];
    int userCount = loadUsers(users);

    if(userCount == 0){
        printf("No such file exists.\n");
        return;
    }

    int id = getIntegerInput("Enter the user ID to update: ");
    if(id == -1){
        printf("Return to menu\n");
        return;
    }
    int check = 0;
    for(int i = 0; i < userCount; i++){
        if(users[i].id == (unsigned int)id){
            getStringInput("Enter new Name: ", users[i].name, sizeof(users[i].name));
            int age = getIntegerInput("Enter new Age: ");
            users[i].age = (unsigned short)age;
            check = 1;
            printf("User updated!\n");
            break;
        }
    }
    
    if(check) {
        saveUsers(users, userCount);
    }
    
    if (!check) {
        printf("User with ID %d not found.\n", id);
    }
}

void deleteUser(){
    User users[MAX_USERS];
    int userCount = loadUsers(users);

    if(userCount == 0){
        printf("No such file exists.\n");
        return;
    }

    int id = getIntegerInput("Enter ID of user want to delete: ");
    if(id == -1){
        printf("Return to menu.\n");
        return;
    }

    int found = 0;
    int newCount = 0;
    
    for(int i = 0; i < userCount; i++){
        if(users[i].id == (unsigned int)id){
            found = 1;
            printf("User deleted successfully!\n");
        } else {
            users[newCount++] = users[i];
        }
    }
    
    if(found) {
        saveUsers(users, newCount);
    }
    
    if(!found){
        printf("User with ID %d not found.\n", id);
    }
}

int main(){
    int choice;
    
    initSystem();
    
    userFile = fopen(FILENAME, "a");
    if(!userFile){
        printf("Error opening file for writing.\n");
        return 1;
    }

    while(1){
        printf("\n====USER MANAGEMENT SYSTEM====\n");
        printf("1. Create User\n");
        printf("2. Read User\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        
        choice = getIntegerInput("Enter your choice: ");

        switch (choice)
        {
            case 1:
                createUser();
                break;
            case 2:
                readUser();
                break;
            case 3:
                updateUser();
                break;
            case 4:
                deleteUser();
                break;
            case 5:
                if(userFile) fclose(userFile);
                exit(0);
                break;
            default:
                printf("Entered invalid choice.\n");
        }
    }
    return 0;
}