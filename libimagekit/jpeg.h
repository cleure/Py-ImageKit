#pragma once

API
ImageKit_Image *
ImageKit_Image_FromJPEG(const char *filepath, REAL scale);

API
int
ImageKit_Image_SaveJPEG(ImageKit_Image *self, const char *filepath);
