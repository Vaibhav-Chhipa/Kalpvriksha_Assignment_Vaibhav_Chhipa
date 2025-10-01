#include<stdio.h>
#include<string.h>

//Stack used for Number
int numbers[100];
//Stack used for operator
char operators[100];
int numTop = -1;
int operatorTop = -1;
int hasError = 0;

int Calculate(char exp[]){
    int len = strlen(exp);
    int i = 0;
    
    while(i < len){
        char c = exp[i];

        if(c == ' '){
            i++;
            continue;
        }

        if((c >= '0' && c <= '9') || (c == '-' && i+1 < len && exp[i+1] >= '0' && exp[i+1] <= '9')){
            int num = 0;
            int isNegative = 0;

            if(c == '-'){
                isNegative = 1;
                i++;
                c = exp[i];
            }

            while(i < len && c >= '0' && c <= '9'){
                num = num * 10 + (c - '0');
                i++;
                if(i < len){
                    c = exp[i];
                }
            }

            if(isNegative){
                num = -1*num;
            }
            
            numTop++;
            numbers[numTop] = num;
            continue;
        }

        if(c == '+' || c == '-' || c == '*' || c == '/'){
            while(operatorTop != -1) {
                char top = operators[operatorTop];

                // Assign precedence to the operator at the top of the stack
                // Multiplication (*) and division (/) have higher precedence (2)
                // Addition (+) and subtraction (-) have lower precedence (1)
                int topPri, curPri;
                if(top == '*' || top == '/')
                    topPri = 2;
                else
                    topPri = 1;
                    
                if(c == '*' || c == '/')
                    curPri = 2;
                else
                    curPri = 1;
                
                if(topPri < curPri)
                    break;
                
                char op = operators[operatorTop];
                operatorTop--;
                
                int b = numbers[numTop];
                numTop--;
                
                int a = numbers[numTop];
                numTop--;
                
                int result;
                if(op == '+'){
                    result = a + b;
                }
                else if(op == '-'){
                    result = a - b;
                }
                else if(op == '*'){
                    result = a * b;
                }
                else if(op == '/'){
                    if(b == 0){
                        hasError = 1;
                        return 0;
                    }
                    result = a / b;
                }
                
                numTop++;
                numbers[numTop] = result;
            }
            
            operatorTop++;
            operators[operatorTop] = c;
        }
        
        i++;
    }
    
    // Process remaining operators
    while(operatorTop != -1){
        char op = operators[operatorTop];
        operatorTop--;  
        
        int b = numbers[numTop];
        numTop--;
        
        int a = numbers[numTop];
        numTop--;
        
        int result;
        if(op == '+'){
            result = a + b;
        }
        else if(op == '-'){
            result = a - b;
        }
        else if(op == '*'){
            result = a * b;
        }
        else if(op == '/'){
            if(b == 0){
                hasError = 1;
                return 0;
            }
            result = a / b;
        }
        
        numTop++;
        numbers[numTop] = result;
    }
    
    return numbers[numTop];
}

int isValid(char exp[]){
    // checkNum is for expecting a Number if there is a number then it should be 1
    // hasNum when finds any Number in the string
    int checkNum = 1;
    int hasNum = 0;

    for(int i = 0;exp[i] != '\0'; i++){
        char c = exp[i];
        if(c == ' '){
            continue;
        }

        if(c >= '0' && c <= '9'){
            if(checkNum == 0){
                return 0;
            }
            hasNum = 1;
            while(exp[i+1] >= '0' && exp[i+1] <= '9'){
                i++;
            }
            checkNum = 0;
        } else if(c == '+' || c == '*' || c == '/'){
            if(checkNum == 1){
                return 0;
            }
            checkNum = 1;
        } else if(c == '-'){
            if(checkNum == 1){
                if(!(exp[i+1] >= '0' && exp[i+1] <= '9')){
                    return 0;
                }
                continue;
            }
            checkNum = 1;
        } else {
            return 0;
        }
    }
    // This ensures that expression end with a number
    if(checkNum == 0 && hasNum == 1){
        return 1;
    }
    return 0;
}

int main(){
    char exp[100];
    scanf("%[^\n]%*c", exp);

    if(isValid(exp) == 0){
        printf("Error: Invalid expression.");
        return 0;
    }

    int ans = Calculate(exp);

    if(hasError == 1){
        printf("Error: Division by zero.");
    }
    else{
        printf("%d", ans);
    }

    return 0;
}