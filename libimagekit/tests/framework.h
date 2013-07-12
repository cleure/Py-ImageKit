#pragma once

#include <stdint.h>

typedef struct DebugMarker {
    char name[512];
    uint32_t index;
} DebugMarker;

void DebugMarker_Inc(DebugMarker *marker);
