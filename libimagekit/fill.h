#pragma once

API
int
ImageKit_Image_Fill(ImageKit_Image *self, REAL *color);

API
int
ImageKit_Image_FillChannel(ImageKit_Image *self, REAL color, DIMENSION channel);

API
int
ImageKit_Image_FillCoords(ImageKit_Image *self, REAL *color, ImageKit_Coords *coords);
