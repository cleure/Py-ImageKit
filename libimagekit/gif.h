#pragma once

API
ImageKit_Image *
ImageKit_Image_FromGIF(const char *filepath, REAL scale);

API
int
ImageKit_Image_SaveGIF(ImageKit_Image *self, const char *filepath);
