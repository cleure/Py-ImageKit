#pragma once

typedef struct ImageKit_Curves {
    DIMENSION data_items;
    REAL *coords;
} ImageKit_Curves;

API
ImageKit_Curves *
ImageKit_Curves_FromBezier(uint32_t samples, uint32_t *xy, size_t xy_items);

API
void
ImageKit_Curves_Delete(ImageKit_Curves *self);
