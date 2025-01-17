#include <string.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    int num1, num2;
    int result = 0;
    char op;
    
    printf("First number: ");
    scanf(" %d", &num1);
    printf("%d\n",num1);

    printf("Operation: ");
    scanf(" %c", &op);
    printf("%c\n", op);

    printf("Second number: ");
    scanf(" %d", &num2);
    printf("%d\n", num2);

    
    switch(op)
    {
        case '+': result = num1 + num2; break;
        case '-': result = num1 - num2; break;
        case '/': result = num1 / num2; break;
        case '*': result = num1 * num2; break;
    }
    printf("Result: %d", result);
}
