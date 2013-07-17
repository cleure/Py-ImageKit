#pragma once

API
ImageKit_Image *
ImageKit_Image_FromPNG(const char *filepath, REAL scale);

API
int
ImageKit_Image_SavePNG(ImageKit_Image *self, const char *filepath);
