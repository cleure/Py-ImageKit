#pragma once

API
int
ImageKit_Image_ApplyRankFilter(
    ImageKit_Image *self,
    DIMENSION matrix_size,
    REAL rank,
    ImageKit_Coords *coords
);
