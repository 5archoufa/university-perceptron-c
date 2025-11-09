#ifndef STACK_H
#define STACK_H
#include <stdlib.h>
#include <stdio.h>

typedef struct Stack Stack;
struct Stack
{
    int count;
    void **content;
    void (*Push)(Stack *stack, void *);
    void *(*Pop)(Stack *stack, void *);
    void *(*Peek)(Stack *stack);
};

void Stack_Free(Stack* stack);
void *Peek(Stack *stack);
void *Pop(Stack *stack, void *item);
void Push(Stack *stack, void *item);
Stack *Stack_Create();
#endif