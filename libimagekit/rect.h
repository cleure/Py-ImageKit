#pragma once

typedef struct ImageKit_Rect {
    DIMENSION x;
    DIMENSION y;
    DIMENSION w;
    DIMENSION h;
} ImageKit_Rect;

API
void
ImageKit_Rect_GetClipped(ImageKit_Image *self, ImageKit_Rect *rect);
