#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_STUDENTS 100

// Error codes for input validation using enum
typedef enum {
    ERROR_NONE = 0,
    ERROR_INVALID_FORMAT,
    ERROR_INVALID_ROLL,
    ERROR_DUPLICATE_ROLL,
    ERROR_INVALID_NAME,
    ERROR_INVALID_MARKS
} ErrorCode;

typedef struct {
    int rollNumber;
    char name[50];
    unsigned short marks1, marks2, marks3;
    int totalMarks;
    float averageMarks;
    char grade;
} Student;

int calculateTotal(int marks1, int marks2, int marks3){
    int totalMarks = 0;
    totalMarks = marks1 + marks2 + marks3;
    return totalMarks;
}

float calculateAverage(int total){
    float average;
    average = (float)total / 3.0; 
    return average;
}

char assignGrade(float average){
    char grade;
    if (average >= 85) {
        grade = 'A';
    } else if (average >= 70) {
        grade = 'B';
    } else if (average >= 50) {
        grade = 'C';
    } else if (average >= 35) {
        grade = 'D';
    } else {
        grade = 'F';
    }
    return grade;
}

void displayPerformancePattern(char grade){
    int stars = 0;
    switch (grade) {
        case 'A':
            stars = 5;
            break;
        case 'B':
            stars = 4;
            break;
        case 'C':
            stars = 3;
            break;
        case 'D':
            stars = 2;
            break;
        default:
            stars = 0;
            break;
    }
    for (int i = 0; i < stars; i++) {
        printf("*");
    }
}

void printStudentDetails(Student s){
    printf("Roll: %d\n", s.rollNumber);
    printf("Name: %s\n", s.name);
    printf("Total: %d\n", s.totalMarks);
    printf("Average: %.2f\n", s.averageMarks);
    printf("Grade: %c\n", s.grade);
}

void printRollNumbersRecursively(Student students[], int current, int total){
    if (current == total) {
        return;
    }
    printf("%d ", students[current].rollNumber);
    printRollNumbersRecursively(students, current + 1, total);
}

int compareRollNumber(const void *a, const void *b) {
    const Student *s1 = (const Student *)a;
    const Student *s2 = (const Student *)b;
     if (s1->rollNumber < s2->rollNumber)
        return -1;
    else if (s1->rollNumber > s2->rollNumber)
        return 1;  
    else
        return 0; 
}

int isRollNumberUnique(Student students[], int count, int rollNumber){
    for (int k = 0; k < count; k++) {
        if (students[k].rollNumber == rollNumber) {
            return 0;
        }
    }
    return 1;
}

ErrorCode validateInput(Student students[], int currentIndex, int scanResult, int roll, char *name, int marks1, int marks2, int marks3){
    // Check if input format is correct
    if (scanResult != 5) {
        return ERROR_INVALID_FORMAT;
    }
    if (roll <= 0) {
        return ERROR_INVALID_ROLL;
    }
    if (!isRollNumberUnique(students, currentIndex, roll)) {
        return ERROR_DUPLICATE_ROLL;
    }
    for (int k = 0; name[k] != '\0'; k++) {
        if (!isalpha(name[k])) {
            return ERROR_INVALID_NAME;
        }
    }
    if (marks1 < 0 || marks1 > 100 || marks2 < 0 || marks2 > 100 || marks3 < 0 || marks3 > 100) {
        return ERROR_INVALID_MARKS;
    }
    return ERROR_NONE;
}

void displayErrorMessage(ErrorCode errorCode){
    switch (errorCode) {
        case ERROR_INVALID_FORMAT:
            printf("Error: Invalid input format. Please re-enter student data:\n");
            break;
        case ERROR_INVALID_ROLL:
            printf("Error: Roll number must be a positive integer. Please re-enter student data:\n");
            break;
        case ERROR_DUPLICATE_ROLL:
            printf("Error: Duplicate roll number found. Please re-enter student data:\n");
            break;
        case ERROR_INVALID_NAME:
            printf("Error: Name must contain only alphabets. Please re-enter student data:\n");
            break;
        case ERROR_INVALID_MARKS:
            printf("Error: Marks must be between 0 and 100! Re-enter details.\n");
            break;
        default:
            break;
    }
}

int main() {
    unsigned short numberOfStudents, i;
    Student students[MAX_STUDENTS];
    char buffer[200];
    printf("========== Student Performance Analyzer ==========\n");
    while (1) {
        printf("Enter number of students: ");
        fgets(buffer, sizeof(buffer), stdin);
        if (sscanf(buffer, "%d", &numberOfStudents) != 1) {
            printf("Error: Invalid input. Please enter a valid number.\n");
            continue;
        }
        if (numberOfStudents < 1 || numberOfStudents > 100) {
            printf("Error: Number of students must be between 1 and 100. Please re-enter.\n");
            continue;
        }

        break;
    }
    printf("\nEnter details for each student in the format:\n");
    
    for (i = 0; i < numberOfStudents; i++) {
        int validInput = 0;
        while (!validInput) {
            printf("Student%d (RollNumber Name Marks1 Marks2 Marks3)\n", i+1);
            fgets(buffer, sizeof(buffer), stdin);
            
            int roll, marks1, marks2, marks3;
            char studentName[50];

            int scanResult = sscanf(buffer, "%d %s %d %d %d", &roll, studentName, &marks1, &marks2, &marks3);
            
            // Validate input and get error code
            ErrorCode errorCode = validateInput(students, i, scanResult, roll, studentName, marks1, marks2, marks3);
            
            if (errorCode != ERROR_NONE) {
                displayErrorMessage(errorCode);
                continue;
            }
            students[i].rollNumber = roll;
            strcpy(students[i].name,studentName);
            students[i].marks1 = marks1;
            students[i].marks2 = marks2;
            students[i].marks3 = marks3;
            students[i].totalMarks = calculateTotal(marks1, marks2, marks3);
            students[i].averageMarks = calculateAverage(students[i].totalMarks);
            students[i].grade = assignGrade(students[i].averageMarks);
            validInput = 1;
        }
    }
    
    qsort(students, numberOfStudents, sizeof(Student), compareRollNumber);
    
    for (i = 0; i < numberOfStudents; i++) {
        printStudentDetails(students[i]);
        if (students[i].grade == 'F') {
            printf("\n");
            continue;
        }
        printf("Performance: ");
        displayPerformancePattern(students[i].grade);
        printf("\n\n");
    }
    printf("List of Roll Numbers (via recursion): ");
    printRollNumbersRecursively(students, 0, numberOfStudents);
    printf("\n");
    return 0;
}