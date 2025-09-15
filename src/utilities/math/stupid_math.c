#include <stdlib.h>
#include <time.h>
#include "utilities/math/stupid_math.h"

float RandomFloat(float min, float max){
    return min + (max - min) * ((double)rand() / RAND_MAX);
}