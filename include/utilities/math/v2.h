#ifndef V2_H
#define V2_H

typedef struct V2 V2;

extern const V2 V2_ONE;
extern const V2 V2_HALF;
extern const V2 V2_ZERO;

struct V2{
    float x, y;
} ;

V2 V2_MUL(V2* a, float b);

#endif