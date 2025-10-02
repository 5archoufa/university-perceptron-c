#include <stdio.h>
#include "utilities/math/v3.h"

const V3 V3_ZERO = {0.0, 0.0, 0.0};
const V3 V3_HALF = {0.5, 0.5, 0.5};
const V3 V3_ONE = {1.0, 1.0, 1.0};

V3 V3_ADD(const V3 *a, const V3 *b) { return (V3){a->x + b->x, a->y + b->y, a->z + b->z}; }
void V3_ADD_FIRST(V3 *a, const V3 *b) { a->x += b->x; a->y += b->y; a->z += b->z; }
void V3_ADD_V2_FIRST(V3 *a, const V2 *b) { a->x += b->x; a->y += b->y; }
V3 V3_SUB(const V3 *a, const V3 *b) { return (V3){a->x - b->x, a->y - b->y, a->z - b->z}; }
V3 V3_MUL(const V3 *a, const V3 *b) { return (V3){a->x * b->x, a->y * b->y, a->z * b->z}; }
V3 V3_DIV(const V3 *a, const V3 *b) { return (V3){a->x / b->x, a->y / b->y, a->z / b->z}; }
V3 V3_NEG(const V3 *a) { return (V3){-a->x, -a->y, -a->z}; }
V3 V3_ADD_V2(const V3 *a, const V2 *b) { return (V3){a->x + b->x, a->y + b->y, a->z}; }