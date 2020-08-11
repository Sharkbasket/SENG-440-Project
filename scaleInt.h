#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define scaleFactor 2^(-30)

int32_t toFixed(float in);
float toFloat(int32_t in);

int32_t toFixed(float in)
{
    return (in*128);
}

float toFloat(int32_t in)
{
    return in/16384;
}
