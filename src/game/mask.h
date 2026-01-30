#pragma once 

enum MaskType {
    MASK_NONE, 
    MASK_STONE, 
    MASK_WIND
};

inline float MaskMoveDuration(MaskType mask) {
    switch (mask) {
        case MASK_STONE: return 0.4f; 
        case MASK_WIND:  return 0.10f; 
        default:         return 0.15f;
    }
}
