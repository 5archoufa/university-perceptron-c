#include "pixel.h"

typedef struct Shape{
    void* self;
    Pixel* (*Draw)(void*);
} Shape;