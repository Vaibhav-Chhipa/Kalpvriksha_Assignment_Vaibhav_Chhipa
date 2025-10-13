#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_STUDENTS 100

// Error codes for input validation using enum
typedef enum {
    ERROR_NONE = 0,
    ERROR_INVALID_FORMAT,
    ERROR_DUPLICATE_ROLL,
    ERROR_INVALID_NAME,
    ERROR_INVALID_MARKS
} ErrorCode;

typedef struct {
    unsigned int rollNumber;
    char name[50];
    unsigned short marks1, marks2, marks3;
    unsigned short totalMarks;
    float averageMarks;
    char grade;
} Student;

unsigned short calculateTotal(unsigned short marks1,unsigned short marks2,unsigned short marks3){
    unsigned short totalMarks = 0;
    totalMarks = marks1 + marks2 + marks3;
    return totalMarks;
}

float calculateAverage(unsigned short total){
    float average;
    average = (float)total / 3; 
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
    printf("Performance: ");
    for (int starIndex = 0; starIndex < stars; starIndex++) {
        printf("*");
    }
}

void printStudentDetails(const Student *studentDetails){
    printf("Roll: %u\n", studentDetails->rollNumber);
    printf("Name: %s\n", studentDetails->name);
    printf("Total: %hu\n", studentDetails->totalMarks);
    printf("Average: %.2f\n", studentDetails->averageMarks);
    printf("Grade: %c\n", studentDetails->grade);
}

void printRollNumbersRecursively(const Student students[], int current, int total){
    if (current == total) {
        return;
    }
    printf("%u ", students[current].rollNumber);
    printRollNumbersRecursively(students, current + 1, total);
}

int compareRollNumber(const void *studentA, const void *studentB) {
    const Student *firstStudent = (const Student *)studentA;
    const Student *secondStudent  = (const Student *)studentB;
     if (firstStudent->rollNumber < secondStudent ->rollNumber)
        return -1;
    else if (firstStudent->rollNumber > secondStudent ->rollNumber)
        return 1;  
    else
        return 0; 
}

int isRollNumberUnique(const Student students[], int count, int rollNumber){
    for (int index = 0; index < count; index++) {
        if (students[index].rollNumber == rollNumber) {
            return 0;
        }
    }
    return 1;
}

ErrorCode validateInput(const Student students[], int currentIndex, int scanResult, const Student * temporaryStudent){
    // Check if input format is correct
    if (scanResult != 5) {
        return ERROR_INVALID_FORMAT;
    }
    if (!isRollNumberUnique(students, currentIndex, temporaryStudent->rollNumber)) {
        return ERROR_DUPLICATE_ROLL;
    }
    for (int index = 0; temporaryStudent->name[index] != '\0'; index++) {
        if (!isalpha(temporaryStudent->name[index])) {
            return ERROR_INVALID_NAME;
        }
    }
    if (temporaryStudent->marks1 > 100 || temporaryStudent->marks2 > 100 || temporaryStudent->marks3 > 100) {
        return ERROR_INVALID_MARKS;
    }
    return ERROR_NONE;
}

void displayErrorMessage(ErrorCode errorCode){
    switch (errorCode) {
        case ERROR_INVALID_FORMAT:
            printf("Error: Invalid input format. Please re-enter student data:\n");
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
    unsigned short numberOfStudents, studentIndex;
    Student students[MAX_STUDENTS];
    char buffer[200];
    printf("========== Student Performance Analyzer ==========\n");
    while (1) {
        printf("Enter number of students: ");
        fgets(buffer, sizeof(buffer), stdin);
        if (sscanf(buffer, "%hu", &numberOfStudents) != 1) {
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
    
    for (studentIndex = 0; studentIndex < numberOfStudents; studentIndex++) {
        int validInput = 0;
        Student temporaryStudent;
        while (!validInput) {
            printf("Student%hu (RollNumber Name Marks1 Marks2 Marks3)\n", studentIndex+1);
            fgets(buffer, sizeof(buffer), stdin);
            

            int scanResult = sscanf(buffer, "%u %s %hu %hu %hu", &temporaryStudent.rollNumber, temporaryStudent.name, &temporaryStudent.marks1, &temporaryStudent.marks2, &temporaryStudent.marks3);
            
            // Validate input and get error code
            ErrorCode errorCode = validateInput(students, studentIndex, scanResult, &temporaryStudent);
            
            if (errorCode != ERROR_NONE) {
                displayErrorMessage(errorCode);
                continue;
            }
            temporaryStudent.totalMarks = calculateTotal(temporaryStudent.marks1, temporaryStudent.marks2, temporaryStudent.marks3);
            temporaryStudent.averageMarks = calculateAverage(temporaryStudent.totalMarks);
            temporaryStudent.grade = assignGrade(temporaryStudent.averageMarks);
            students[studentIndex] = temporaryStudent;
            validInput = 1;
        }
    }
    
    qsort(students, numberOfStudents, sizeof(Student), compareRollNumber);
    
    for (studentIndex = 0; studentIndex < numberOfStudents; studentIndex++) {
        printStudentDetails(&students[studentIndex]);
        if (students[studentIndex].grade == 'F') {
            printf("\n");
            continue;
        }
        displayPerformancePattern(students[studentIndex].grade);
        printf("\n\n");
    }
    printf("List of Roll Numbers (via recursion): ");
    printRollNumbersRecursively(students, 0, numberOfStudents);
    printf("\n");
    return 0;
}