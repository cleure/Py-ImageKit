
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include "imagekit.h"

API
void
ImageKit_Cleanup()
{
    ImageKit_CleanupError();
}
