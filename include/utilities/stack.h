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
    void *(*Pop)(Stack *stack);
    void *(*Peek)(Stack *stack);
};

void FreeStack(Stack* stack);
void *Peek(Stack *stack);
void *Pop(Stack *stack);
void Push(Stack *stack, void *item);
Stack *CreateStack();
#endif