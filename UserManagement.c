#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#define FILENAME "users.txt"
#define MAX_USERS 100

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

int getStringInput(const char *text, char *userName, size_t size){
    while(1){
        printf("%s", text);
        if(fgets(userName, size, stdin) != NULL){
            userName[strcspn(userName, "\n")] = '\0';
            
            // Check if empty (allow cancel)
            if(strlen(userName) == 0) return 0;
            
            // Validate: only letters and spaces
            int valid = 1;
            for(int i = 0; userName[i] != '\0'; i++){
                if(!isalpha(userName[i]) && userName[i] != ' '){
                    valid = 0;
                    break;
                }
            }
            
            if(valid) return 1;
            printf("Invalid name! Only letters and spaces allowed. (Enter to cancel)\n");
        }
    }
}

void createUser(){
    FILE *file;
    User newUser;

    currentId++;
    newUser.id = currentId;

    printf("Your Unique ID is: %u\n", newUser.id);

    if(!getStringInput("Enter Name: ", newUser.name, sizeof(newUser.name))){
        printf("Operation cancelled.\n");
        currentId--;
        return;
    }

    int age = getIntegerInput("Enter Age: ");
    if(age == -1){
        printf("Operation cancelled.\n");
        currentId--;
        return;
    }
    newUser.age = (unsigned short)age;
    
    // Open file once in append mode
    file = fopen(FILENAME, "a");
    if(!file){
        printf("Error in opening File\n");
        currentId--;
        return;
    }

    fprintf(file, "%u,%s,%hu\n", newUser.id, newUser.name, newUser.age);
    fclose(file);

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
        printf("Operation cancelled.\n");
        return;
    }
    
    int check = 0;
    for(int i = 0; i < userCount; i++){
        if(users[i].id == (unsigned int)id){
            if(!getStringInput("Enter new Name: ", users[i].name, sizeof(users[i].name))){
                printf("Operation cancelled.\n");
                return;
            }
            
            int age = getIntegerInput("Enter new Age: ");
            if(age == -1){
                printf("Operation cancelled.\n");
                return;
            }
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
        printf("Operation cancelled.\n");
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

    while(1){
        printf("\n====USER MANAGEMENT SYSTEM====\n");
        printf("1. Create User\n");
        printf("2. Read User\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        
        choice = getIntegerInput("Enter your choice: ");
        if(choice == -1) continue;

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
                exit(0);
                break;
            default:
                printf("Entered invalid choice.\n");
        }
    }
    return 0;
}