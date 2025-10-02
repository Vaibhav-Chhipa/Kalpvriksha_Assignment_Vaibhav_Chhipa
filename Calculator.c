#include<stdio.h>
#include<string.h>

//Stack used for Number
int numbers[100];
//Stack used for operator
char operators[100];
int numTop = -1;
int operatorTop = -1;
int hasError = 0;

int execute(int a, int b, char op){
    int result;
    if(op == '+'){
        result = a + b;
    } else if(op == '-'){
        result = a - b;
    } else if(op == '*'){
        result = a * b;
    } else if(op == '/'){
        if(b == 0){
            hasError = 1;
            return 0;
        }
        result = a / b;
    }
    return result;
}

int Calculate(char exp[]){
    int len = strlen(exp);
    int i = 0;
    int expectNumber = 1;
    
    while(i < len){
        char c = exp[i];

        if(c == ' '){
            i++;
            continue;
        }

        if((c >= '0' && c <= '9') || (c == '-' && expectNumber)){
            int num = 0;
            int isNegative = 0;

            if(c == '-'){
                isNegative = 1;
                i++;
                if(i >= len || !(exp[i] >= '0' && exp[i] <= '9')){
                    break;
                }
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
                num = -1 * num;
            }
            
            numTop++;
            numbers[numTop] = num;
            expectNumber = 0;
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
                
                int b = numbers[numTop];
                numTop--;
                
                int a = numbers[numTop];
                numTop--;
                
                int result = execute(a, b, top);
                operatorTop--;
                
                numTop++;
                numbers[numTop] = result;
            }
            
            operatorTop++;
            operators[operatorTop] = c;
            expectNumber = 1;
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
        
        int result = execute(a, b, op);
        
        numTop++;
        numbers[numTop] = result;
    }
    
    return numbers[numTop];
}

int isValid(char exp[]){
    // expectNumber is for expecting a Number if there is a number then it should be 1
    // hasNum when finds any Number in the string
    int expectNumber = 1;
    int hasNum = 0;

    for(int i = 0; exp[i] != '\0'; i++){
        char c = exp[i];
        if(c == ' '){
            continue;
        }

        if(c >= '0' && c <= '9'){
            if(expectNumber == 0){
                return 0;
            }
            hasNum = 1;
            while(exp[i+1] >= '0' && exp[i+1] <= '9'){
                i++;
            }
            expectNumber = 0;
        } else if(c == '+' || c == '*' || c == '/'){
            if(expectNumber == 1){
                return 0;
            }
            expectNumber = 1;
        } else if(c == '-'){
            if(expectNumber == 1){
                // Minus as negative sign, must be followed by digit
                if(i+1 >= strlen(exp) || !(exp[i+1] >= '0' && exp[i+1] <= '9')){
                    return 0;
                }
                continue;
            }
            // Minus as operator
            expectNumber = 1;
        } else {
            return 0;
        }
    }
    // This ensures that expression end with a number
    if(expectNumber == 0 && hasNum == 1){
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