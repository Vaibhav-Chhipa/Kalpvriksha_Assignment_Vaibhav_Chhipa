#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define FILENAME "users.txt"


typedef struct {
    int id;
    char name[50];
    int age;
} User;

void createUser(){
    FILE *file;
    User newUser;
    int maxID = 0;
    file = fopen(FILENAME, "r");
    if(file == NULL){
        maxID = 1;
    }
    while(fscanf(file, "%d,%[^,],%d\n", &newUser.id, newUser.name, &newUser.age) == 3){
        if(newUser.id > maxID){
            maxID = newUser.id;
        }
        maxID++;
    }
    newUser.id = maxID;
    printf("Your Unique ID is: %d\n", newUser.id);
    

    if(file != NULL){
        User u;
        while(fscanf(file, "%d,%[^,],%d\n", &u.id, u.name, &u.age) == 3){
            if(u.id == newUser.id){
                printf("Error! ID %d already exists. Please use unique ID.\n", newUser.id);
                fclose(file);
                return;
            }
        }
        fclose(file);
    }
    printf("Enter Name: ");
    scanf(" %[^\n]", newUser.name);
    printf("Enter Age: ");
    scanf("%d", &newUser.age);

    file = fopen(FILENAME, "a");
    if(!file){
        printf("Error in opening FIle");
        return;
    }

    fprintf(file, "%d,%s,%d\n", newUser.id, newUser.name, newUser.age);
    fclose(file);

    printf("User Added");
}

void readUser(){
    FILE *file = fopen(FILENAME, "r");
    if(file == NULL){
        printf("No such file exists.");
        return;
    }

    User user;

    while (fscanf(file, "%d,%[^,],%d\n", &user.id, user.name, &user.age) == 3) {
    printf("\nUser ID : %d \nUser Name : %s \nUser Age : %d\n", user.id, user.name, user.age);
    }
    
    fclose(file);
}

void updateUser(){
    FILE *file = fopen(FILENAME, "r");
    if(file == NULL){
        printf("No such file exists.");
        return;
    }

    int id;
    printf("Enter the user ID to update: ");
    scanf("%d", &id);

    FILE *temp = fopen("temp.txt", "w");

    User u;
    int check = 0;
    while(fscanf(file, "%d,%[^,],%d\n", &u.id, u.name, &u.age) == 3){
        if(u.id == id){
            printf("Enter new name: ");
            scanf(" %[^\n]", u.name);
            printf("Enter new Age: ");
            scanf("%d", &u.age);
            check = 1;
            printf("User updated!\n");
        }
        fprintf(temp, "%d,%s,%d\n", u.id, u.name, u.age);
    }

    fclose(file);
    fclose(temp);
    
    remove(FILENAME);
    rename("temp.txt", FILENAME);
    
    if (!check) {
        printf("User with ID %d not found.\n", id);
    }
}

void deleteUser(){
    FILE *file = fopen(FILENAME, "r");
    if(file == NULL){
        printf("No such file exists!");
        return;
    }

    int id;
    printf("Enter ID of user want to delete: ");
    scanf("%d", &id);

    FILE *temp = fopen("temp.txt", "w");
    
    User u;
    int found = 0;
    
    while(fscanf(file, "%d,%[^,],%d\n", &u.id, u.name, &u.age) == 3){
        if(u.id == id){
            found = 1;
            printf("User deleted successfully!\n");
        } else {
            fprintf(temp, "%d,%s,%d\n", u.id, u.name, u.age);
        }
    }
    
    fclose(file);
    fclose(temp);
    
    remove(FILENAME);
    rename("temp.txt", FILENAME);
    
    if(!found){
        printf("User with ID %d not found.\n", id);
    }
}


int main(){
    int choice;

    while(1){
        printf("\n====USER MANAGEMENT SYSTEM====\n");
        printf("1. Create User\n");
        printf("2. Read User\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

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
                printf("Entered invalid number.\n");
        }
    }
    return 0;
}