#ifndef ZOOMER_VECTOR_H_
#define ZOOMER_VECTOR_H_

#include <math.h>

typedef struct Vector {
    float x;
    float y;
} Vector;

Vector VectorAdd(Vector v1, Vector v2)
{
    Vector result = { v1.x + v2.x, v1.y + v2.y };
    return result;
}

void VectorAddMut(Vector v1, Vector v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
}

Vector VectorSubtract(Vector v1, Vector v2)
{
    Vector result = { v1.x - v2.x, v1.y - v2.y };
    return result;
}

void VectorSubtractMut(Vector v1, Vector v2)
{
    v1.x -= v2.x;
    v1.y -= v2.y;
}

Vector VectorScale(Vector v, float f)
{
    Vector result = { v.x * f, v.y * f };
    return result;
}

Vector VectorMultiply(Vector v1, Vector v2)
{
    Vector result = { v1.x * v2.x, v1.y * v2.y };
    return result;
}

Vector VectorDivide(Vector v1, Vector v2)
{
    Vector result = { v1.x / v2.x, v1.y / v2.y };
    return result;
}

float VectorLength(Vector v)
{
    float result = sqrtf((v.x * v.x) + (v.y * v.y));
    return result;
}

Vector VectorNormalize(Vector v)
{
    float length = VectorLength(v);
    if (fabs(length) < 1e-5f) {
        Vector result = { 0.0f, 0.0f };
        return result;
    } else {
        Vector result = VectorScale(v, 1 / length);
        return result;
    }
}

#endif // ZOOMER_VECTOR_H_
