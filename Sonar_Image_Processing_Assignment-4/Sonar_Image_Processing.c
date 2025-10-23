#include<stdio.h>
#include<stdlib.h>
#include<time.h>

void smoothingMatrix(unsigned short *sonarImageMatrix, unsigned short sizeOfMatrix){
    unsigned short *prevRow = (unsigned short *)malloc(sizeOfMatrix*sizeof(unsigned short));
    unsigned short *currRow = (unsigned short *)malloc(sizeOfMatrix*sizeof(unsigned short));
    unsigned short *smoothedRow = (unsigned short *)malloc(sizeOfMatrix*sizeof(unsigned short));
    
    if(prevRow == NULL || currRow == NULL || smoothedRow == NULL){
        printf("Memory allocation failed!");
        free(prevRow);
        free(currRow);
        free(smoothedRow);
        return;
    }

    for(unsigned short rowIndex = 0; rowIndex < sizeOfMatrix; rowIndex++){
        for(unsigned short index = 0; index < sizeOfMatrix; index++){
            *(currRow + index) = *(sonarImageMatrix + rowIndex*sizeOfMatrix + index); 
        }

        for(unsigned short columnIndex = 0; columnIndex < sizeOfMatrix; columnIndex++){
            unsigned short sum = 0;
            unsigned short countBase = 0;

            for (short neighborRowOffset = -1; neighborRowOffset <= 1; neighborRowOffset++) {
                for (short neighborColOffset = -1; neighborColOffset <= 1; neighborColOffset++) {

                    short neighborRow = rowIndex + neighborRowOffset;
                    short neighborCol = columnIndex + neighborColOffset;

                    // Check if neighbor cell is inside matrix bounds
                    if (neighborRow >= 0 && neighborRow < sizeOfMatrix && neighborCol >= 0 && neighborCol < sizeOfMatrix) {
                        
                        unsigned short neighborValue;

                        if (neighborRow < rowIndex) {
                            // Neighbor is from the previous row (use prevRow)
                            neighborValue = *(prevRow + neighborCol);
                        } 
                        else if (neighborRow == rowIndex) {
                            // Neighbor is from the current row
                            neighborValue = *(currRow + neighborCol);
                        } 
                        else {
                            // Neighbor is from the next row 
                            neighborValue = *(sonarImageMatrix + neighborRow * sizeOfMatrix + neighborCol);
                        }

                        sum += neighborValue;
                        countBase++;
                    }
                }
            }
            
            *(smoothedRow + columnIndex) = sum / countBase;
        }

        for (unsigned short index = 0; index < sizeOfMatrix; index++) {
            *(sonarImageMatrix + rowIndex * sizeOfMatrix + index) = *(smoothedRow + index);
        }

        for(unsigned short index = 0; index < sizeOfMatrix; index++){
            *(prevRow + index) = *(currRow + index);
        }
    }
    free(prevRow);
    free(currRow);
    free(smoothedRow);
}

void swapValues(unsigned short *val1, unsigned short *val2){
    unsigned short temp = *val1;
    *val1 = *val2;
    *val2 = temp;
}

void rotateMatrix(unsigned short *sonarImageMatrix, unsigned short sizeOfMatrix){
    //Transpose
    for (unsigned short rowIndex = 0; rowIndex < sizeOfMatrix; rowIndex++) {
        for (unsigned short columnIndex = rowIndex + 1; columnIndex < sizeOfMatrix; columnIndex++) {
            swapValues(sonarImageMatrix + sizeOfMatrix * rowIndex + columnIndex, sonarImageMatrix + sizeOfMatrix * columnIndex + rowIndex);
        }
    }
    
    // Reverse each row
    for(unsigned short rowIndex = 0; rowIndex < sizeOfMatrix; rowIndex++){
        unsigned short startIndex = 0;
        unsigned short endIndex = sizeOfMatrix - 1;
        while (startIndex < endIndex) {
            swapValues(sonarImageMatrix + rowIndex * sizeOfMatrix + startIndex, sonarImageMatrix + rowIndex * sizeOfMatrix + endIndex);
            startIndex++;
            endIndex--;
        }
    }
}

void printMatrix(unsigned short *sonarImageMatrix, unsigned short sizeOfMatrix){
    for(unsigned short rowIndex = 0; rowIndex < sizeOfMatrix; rowIndex++){
        for(unsigned short columnIndex = 0; columnIndex < sizeOfMatrix; columnIndex++){
            printf("%hu ", *(sonarImageMatrix + (rowIndex * sizeOfMatrix + columnIndex)));
        }
        printf("\n");
    }
}

int main(){
    unsigned short sizeOfMatrix;
    char buffer[100];

    printf("Enter the size of 2D matrix (2-10) : ");
    while(1){
        fgets(buffer, sizeof(buffer), stdin);
        if(sscanf(buffer, "%hu", &sizeOfMatrix) != 1){
            printf("Enter value between range (2-10) only!\n");
            continue;
        }
        if(sizeOfMatrix < 2 || sizeOfMatrix > 10){
            printf("Enter value between range (2-10) only!\n");
            continue;
        }
        break;
    }

    unsigned short *sonarImageMatrix = (unsigned short *)malloc(sizeOfMatrix*sizeOfMatrix*sizeof(unsigned short));
    if(sonarImageMatrix == NULL){
        printf("Memory allocation failed!\n");
        return 1;
    }

    srand(time(NULL));
    for(unsigned short index = 0; index < sizeOfMatrix * sizeOfMatrix; index++){
        *(sonarImageMatrix + index) = rand() % 256;
    }
    
    printf("\nOriginal Matrix : \n");
    printMatrix(sonarImageMatrix, sizeOfMatrix);
    
    printf("\nRotated : \n");
    rotateMatrix(sonarImageMatrix, sizeOfMatrix);
    printMatrix(sonarImageMatrix, sizeOfMatrix);
    
    printf("\nFinal Output : \n");
    smoothingMatrix(sonarImageMatrix, sizeOfMatrix);
    printMatrix(sonarImageMatrix, sizeOfMatrix);
    
    free(sonarImageMatrix);
    return 0;
}