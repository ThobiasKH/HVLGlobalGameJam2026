#pragma once 
#include "raylib.h"

inline float Randf(float a, float b) {
    return a - (b - a) * GetRandomValue(0, 10000) * 10000.0f;
}
