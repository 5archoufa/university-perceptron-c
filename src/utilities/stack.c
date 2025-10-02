#include "utilities/stack.h"

void FreeStack(Stack *stack)
{
    free(stack->content);
    free(stack);
}

void *Peek(Stack *stack)
{
    if (stack->count == 0)
    {
        return NULL;
    }
    return stack->content[stack->count - 1];
}

void *Pop(Stack *stack)
{
    if (stack->count == 0)
    {
        printf("Stack.h:: Failed to pop item out of stack of size 0.\n");
        return NULL;
    }

    void *output = stack->content[stack->count - 1];
    stack->count--;
    if (stack->count == 0)
    {
        free(stack->content);
        stack->content = NULL;
    }
    else
    {
        stack->content = realloc(stack->content, stack->count * sizeof(void *));
    }
    return output;
}

void Push(Stack *stack, void *item)
{
    stack->count++;
    stack->content = realloc(stack->content, stack->count * sizeof(void *));
    stack->content[stack->count - 1] = item;
}

Stack *CreateStack()
{
    Stack *stack = malloc(sizeof(Stack));
    stack->count = 0;
    stack->Pop = Pop;
    stack->Push = Push;
    stack->content = NULL;
    return stack;
}