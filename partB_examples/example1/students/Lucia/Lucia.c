int main(int argc, char* argv[])
{
    int num1, num2;
    double result = 0;
    char op;
    
    printf("First number:\n");
    scanf("%d", &num1);
    
    printf("Operation:\n");
    scanf("%c", &op);
    
    printf("Second number:\n");
    scanf("%d", &num2);
    
    switch(op)
    {
        case '+': result = num1 + num2; break;
        case '-': result = num1 - num2; break;
        case '/': result = num1 / num2; break;
        case '*': result = num1 * num2; break;
    }
    printf("Result: %f", result);
}
