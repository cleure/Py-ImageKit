#pragma once

/**
* Apply matrix (point filter). Matrix must be the same size as the number of
* channels the image has. Each channel in the image will be multiplied by its
* corresponding value in the matrix array. Eg:
*
*   RGB[255, 255, 255] * matrix[1.0, 0.5, 0.5] = RGB[255, 127, 127]
*
* The coords parameters is optional. If supplied, then only the pixels described
* by "ImageKit_Coords *coords" will be filtered.
*
* @param    ImageKit_Image *self
* @param    REAL *matrix
* @param    ImageKit_Coords *coords
* @return   <= 0 on error
**/
API
int
ImageKit_Image_ApplyMatrix(ImageKit_Image *self, REAL *matrix, ImageKit_Coords *coords);

/**
* Apply 2D matrix (point filter). Matrix must be of size channels*channels. Eg:
*
*    REAL matrix[] = {
*        0.299,   0.587,   0.114,
*        0.596,  -0.275,  -0.321,
*        0.212,  -0.523,   0.311,
*    };
*
* Note: values are NOT clamped. This function is mainly useful for converting
* between RGB encoding schemes, such as YPbPr, YUV, YIQ, etc.
*
* @param    ImageKit_Image *self
* @param    REAL *matrix
* @return   <= 0 on error
**/
API
int
ImageKit_Image_ApplyMatrix2D(ImageKit_Image *self, REAL *matrix);
