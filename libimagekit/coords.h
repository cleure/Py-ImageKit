#pragma once

typedef struct Coordinates {
    size_t data_size;
    size_t data_items;
    size_t data_index;
    DIMENSION *coords;
} ImageKit_Coords;

API
ImageKit_Coords *
ImageKit_Coords_New(size_t items);

API
void
ImageKit_Coords_Delete(ImageKit_Coords *self);

API
int
ImageKit_Coords_Resize(ImageKit_Coords *self, size_t items);

API
int
ImageKit_Coords_Append(ImageKit_Coords *self, DIMENSION x, DIMENSION y);
