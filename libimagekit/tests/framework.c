
#include <stdio.h>
#include "tests/framework.h"

void DebugMarker_Inc(DebugMarker *marker)
{
    fprintf(stderr, "%s: %u\n", marker->name, marker->index++);
}
