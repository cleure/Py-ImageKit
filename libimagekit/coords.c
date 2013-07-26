
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include "imagekit.h"

API
ImageKit_Coords *
ImageKit_Coords_New(size_t items)
{
    ImageKit_Coords *self;
    DIMENSION *coords;
    size_t size;
    
    self = malloc(sizeof(*self));
    if (!self) {
        ImageKit_SetError(ImageKit_MemoryError, "Failed to allocate memory");
        return NULL;
    }
    
    size = items * 2 * sizeof(*coords);
    coords = malloc(size);
    if (!coords) {
        free(self);
        ImageKit_SetError(ImageKit_MemoryError, "Failed to allocate memory");
        return NULL;
    }
    
    memset(coords, 0, size);
    
    self->data_size = size;
    self->data_items = items;
    self->coords = coords;
    self->data_index = 0;
    
    return self;
}

API
void
ImageKit_Coords_Delete(ImageKit_Coords *self)
{
    free(self->coords);
    free(self);
}

API
ImageKit_Coords *
ImageKit_Coords_FromRect(ImageKit_Rect *rect)
{
    DIMENSION x, y, ex, ey;
    ImageKit_Coords *coords;
    
    coords = ImageKit_Coords_New(rect->w * rect->h);
    if (!coords) {
        return NULL;
    }
    
    ex = rect->x + rect->w;
    ey = rect->y + rect->h;
    
    for (y = rect->y; y < ey; y++) {
        for (x = rect->x; x < ex; x++) {
            if (!ImageKit_Coords_Append(coords, x, y)) {
                ImageKit_Coords_Delete(coords);
                return NULL;
            }
        }
    }
    
    return coords;
}

API
int
ImageKit_Coords_Resize(ImageKit_Coords *self, size_t items)
{
    DIMENSION *coords;
    size_t size, copy_size;
    
    size = items * 2 * sizeof(*coords);
    coords = malloc(size);
    if (!coords) {
        ImageKit_SetError(ImageKit_MemoryError, "Failed to allocate memory");
        return 0;
    }
    
    if (size < self->data_size) {
        copy_size = size;
    } else {
        copy_size = self->data_size;
    }
    
    memset(coords, 0, size);
    memcpy(coords, self->coords, copy_size);
    
    free(self->coords);
    self->data_size = size;
    self->data_items = items;
    self->coords = coords;
    
    if (!(self->data_index < self->data_items)) {
        self->data_index = self->data_items;
    }
    
    return 1;
}

API
int
ImageKit_Coords_Append(ImageKit_Coords *self, DIMENSION x, DIMENSION y)
{
    if (!(self->data_index < self->data_items)) {
        ImageKit_SetError(ImageKit_IndexError, "Maximum capacity reached");
        return 0;
    }
    
    self->coords[self->data_index * 2 + 0] = x;
    self->coords[self->data_index * 2 + 1] = y;
    self->data_index++;
    
    return 1;
}
