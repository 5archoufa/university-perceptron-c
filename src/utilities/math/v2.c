#include "utilities/math/v2.h"
const V2 V2_ONE = {1.0, 1.0};
const V2 V2_HALF = {0.5, 0.5};
const V2 V2_ZERO = {0.0, 0.0};

V2 V2_MUL(V2* a, float b){
    return (V2){a->x * b, a->y * b};
}