#include "utilities/stack.h"

void Stack_Free(Stack *stack)
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

void *Pop(Stack *stack, void *item)
{
    if (stack->count == 0)
    {
        printf("Stack.h:: Failed to pop item out of stack of size 0.\n");
        return NULL;
    }

    if (item == NULL && stack->count > 0)
    {
        item = stack->content[stack->count - 1];
    }

    for (int i = stack->count - 1; i >= 0; i--)
    {
        if (stack->content[i] == item)
        {
            // Shift remaining items down
            for (int j = i; j < stack->count; j++)
            {
                stack->content[j] = stack->content[j + 1];
            }
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
            return item;
        }
    }
    return NULL;
}

void Push(Stack *stack, void *item)
{
    for(int i = 0;i< stack->count;i++) {
        if(stack->content[i] == item) {
            return;
        }
    }
    stack->count++;
    stack->content = realloc(stack->content, stack->count * sizeof(void *));
    stack->content[stack->count - 1] = item;
}

Stack *Stack_Create()
{
    Stack *stack = malloc(sizeof(Stack));
    stack->count = 0;
    stack->Pop = Pop;
    stack->Push = Push;
    stack->content = NULL;
    return stack;
}