#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<limits.h>

#define MAX_EXPRESSION_LEN 100
#define MAX_STACK_SIZE (MAX_EXPRESSION_LEN / 2 + 1)

#define SUCCESS 0
#define ERROR_INVALID 1
#define ERROR_DIV_ZERO 2
#define ERROR_OVERFLOW 3
#define ERROR_UNDERFLOW 4
#define ERROR_TOO_LONG 5

int getPrecedence(char operator){
    if(operator == '*' || operator == '/')
        return 2;
    else
        return 1;
}

int willOverflow(long long number1, long long number2, char operator){
    if(operator == '+'){
        if(number2 > 0 && number1 > LLONG_MAX - number2) return 1;
        if(number2 < 0 && number1 < LLONG_MIN - number2) return 1;
    } else if(operator == '-'){
        if(number2 < 0 && number1 > LLONG_MAX + number2) return 1;
        if(number2 > 0 && number1 < LLONG_MIN + number2) return 1;
    } else if(operator == '*'){
        if(number1 == 0 || number2 == 0) return 0;
        if(number1 > 0){
            if(number2 > 0 && number1 > LLONG_MAX / number2) return 1;
            if(number2 < 0 && number2 < LLONG_MIN / number1) return 1;
        } else {
            if(number2 > 0 && number1 < LLONG_MIN / number2) return 1;
            if(number2 < 0 && number1 < LLONG_MAX / number2) return 1;
        }
    } else if(operator == '/'){
        // Division overflow: LLONG_MIN / -1
        if(number1 == LLONG_MIN && number2 == -1) return 1;
    }
    return 0;
}

int execute(long long number1, long long number2, char operator, long long *result){
    if(operator == '+'){
        if(willOverflow(number1, number2, operator)) return ERROR_OVERFLOW;
        *result = number1 + number2;
    } else if(operator == '-'){
        if(willOverflow(number1, number2, operator)) return ERROR_OVERFLOW;
        *result = number1 - number2;
    } else if(operator == '*'){
        if(willOverflow(number1, number2, operator)) return ERROR_OVERFLOW;
        *result = number1 * number2;
    } else if(operator == '/'){
        if(number2 == 0) return ERROR_DIV_ZERO;
        if(willOverflow(number1, number2, operator)) return ERROR_OVERFLOW;
        *result = number1 / number2;
    }
    return SUCCESS;
}

int Calculate(const char exp[], long long *result){
    long long numbers[MAX_STACK_SIZE];
    char operators[MAX_STACK_SIZE];
    int numTop = -1;
    int operatorTop = -1;

    int exprLength = strlen(exp);
    int i = 0;
    int lastWasOperator = 1;
    int hasNumber = 0;
    
    while(i < exprLength){
        char c = exp[i];

        if(isspace(c)){
            i++;
            continue;
        }

        if(isdigit(c) || ((c == '-' || c == '+') && lastWasOperator && hasNumber == 0)){
            if(!lastWasOperator) return ERROR_INVALID;
            
            long long number = 0;
            int leadingNegative = 0;

            //Handle leading sign(+ or -)
            if(c == '-' || c == '+'){
                if(c == '-'){
                    leadingNegative = 1;
                }
                i++;
                if(i >= exprLength || !isdigit(exp[i])){
                    return ERROR_INVALID;
                }
                c = exp[i];
            }

            while(i < exprLength && isdigit(exp[i])){
                if(number > LLONG_MAX / 10) return ERROR_OVERFLOW;
                long long digit = exp[i] - '0';
                if(number > (LLONG_MAX - digit) / 10) return ERROR_OVERFLOW;
                number = number * 10 + digit;
                i++;
            }

            if(leadingNegative){
                if(number > LLONG_MAX){
                    return ERROR_OVERFLOW;
                }
                number = -number;
            }
            
            if(numTop >= MAX_STACK_SIZE - 1) return ERROR_OVERFLOW;
            numTop++;
            numbers[numTop] = number;
            lastWasOperator = 0;
            hasNumber = 1;
            continue;
        }

        if(c == '+' || c == '-' || c == '*' || c == '/'){
            if(lastWasOperator) return ERROR_INVALID;
            
            while(operatorTop != -1) {
                char topStackOperator = operators[operatorTop];
                int topPriority = getPrecedence(topStackOperator);
                int curPriority = getPrecedence(c);
                
                if(topPriority < curPriority)
                    break;
                
                if(numTop < 1) return ERROR_UNDERFLOW;
                long long number2 = numbers[numTop];
                numTop--;
                
                long long number1 = numbers[numTop];
                numTop--;
                
                long long res;
                int err = execute(number1, number2, topStackOperator, &res);
                if(err != SUCCESS) return err;
                
                operatorTop--;
                
                if(numTop >= MAX_STACK_SIZE - 1) return ERROR_OVERFLOW;
                numTop++;
                numbers[numTop] = res;
            }
            
            if(operatorTop >= MAX_STACK_SIZE - 1) return ERROR_OVERFLOW;
            operatorTop++;
            operators[operatorTop] = c;
            lastWasOperator = 1;
        } else {
            return ERROR_INVALID;
        }
        i++;
    }
    
    if(lastWasOperator || !hasNumber) return ERROR_INVALID;
    
    // Process remaining operators
    while(operatorTop != -1){
        char operator = operators[operatorTop];
        operatorTop--;

        if(numTop < 1) return ERROR_INVALID;
        long long number2 = numbers[numTop];
        numTop--;
        
        long long number1 = numbers[numTop];
        numTop--;
        
        long long res;
        int err = execute(number1, number2, operator, &res);
        if(err != SUCCESS) return err;
        
        if(numTop >= MAX_STACK_SIZE - 1) return ERROR_OVERFLOW;
        numTop++;
        numbers[numTop] = res;
    }
    
    if(numTop != 0) return ERROR_INVALID;
    *result = numbers[numTop];
    return SUCCESS;
}

void displayMenu(){
    printf("\n---Calculator Menu---\n");
    printf("1. Evaluate Expression\n");
    printf("2. Exit\n");
}

int main(){
    
    int choice;
    do{
        displayMenu();
        printf("Enter your choice : ");
        scanf("%d", &choice);
        getchar();
        
        if(choice == 1){
            // Buffer for expression: MAX length + '\n' + '\0' for fgets safety
            char exp[MAX_EXPRESSION_LEN + 2];

            printf("Enter the Expression \n");
            fgets(exp, sizeof(exp), stdin);

            int expLength = strlen(exp);
            // If fgets, not reads the entire line (no '\n') and not at EOF,
            // the input is too long. Print error.
            if(expLength > 0 && exp[expLength - 1] != '\n' && !feof(stdin)){
                printf("Error: Expression too long.");
                int c;
                while((c = getchar()) != '\n' && c != EOF);
                continue;
            }
            
            if(expLength > 0 && exp[expLength - 1] == '\n'){
                exp[expLength - 1] = '\0';
                expLength--;
            }
            
            if(expLength == 0){
                printf("Error: Invalid expression.");
                continue;
            }
            long long ans;
            int errorCode = Calculate(exp, &ans);
        
            if(errorCode == ERROR_INVALID){
                printf("\nError: Invalid expression.\n");
            } else if(errorCode == ERROR_DIV_ZERO){
                printf("Error: Division by zero.\n");
            } else if(errorCode == ERROR_OVERFLOW){
                printf("Error: Numeric overflow.\n");
            } else if(errorCode == ERROR_UNDERFLOW){
                printf("Error: Stack underflow.\n");
            } else {
                printf("Result : %lld\n", ans);
            }
        } else if(choice == 2){
            printf("Exiting\n");
        } else {
            printf("\nInvalid choice!\n");
        }
    } while(choice != 2);
    
    return 0;
}