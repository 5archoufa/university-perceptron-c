#include "ui/x11/pixel.h"
#include <stdlib.h>
#include <stdio.h>

void FreePixels(Pixel* head){
    int count = 0;
    Pixel* p = NULL;
    while(head != NULL){
        p = head->next;
        free(head);
        head = p;
        count++;
    }
    printf("Freed %d pixels\n", count);
}