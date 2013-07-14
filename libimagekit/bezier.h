#pragma once

typedef struct ImageKit_Bezier {
    DIMENSION data_items;
    REAL *coords;
} ImageKit_Bezier;

API
ImageKit_Bezier *
ImageKit_Bezier_New(uint32_t samples, uint32_t *xy, size_t xy_items);

API
void
ImageKit_Bezier_Delete(ImageKit_Bezier *self);
