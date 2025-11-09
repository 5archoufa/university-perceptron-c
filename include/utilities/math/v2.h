#ifndef V2_H
#define V2_H

typedef struct V2 V2;
typedef struct V2_INT V2_INT;

extern const V2 V2_ONE;
extern const V2 V2_HALF;
extern const V2 V2_ZERO;

struct V2{
    float x, y;
} ;

struct V2_INT{
    int x, y;
} ;

V2 V2_SCALE(V2* a, float b);
V2 V2_MUL(V2 *a, V2 *b);
V2 V2_SUB(V2 a, V2 b);
V2 V2_DIV(V2 *a, V2 *b);
V2 V2_SIZE(V2 *a, V2 *b);
V2 V2_CENTER(V2 *a, V2 *b);
V2 V2_NORM(V2 a);
float V2_DOT(V2 *a, V2 *b);
float V2_INT_DOT(V2_INT *a, V2_INT *b);
float V2_MAGNITUDE(V2 *a);
float V2_INT_MAGNITUDE(V2_INT *a);
V2 V2_Direction(float x, float y);
V2 V2_Rand(float magnitude);

#endif