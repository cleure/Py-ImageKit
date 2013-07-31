#pragma once

/*

NOTE:
kernel_size is the square root of the number of elements in kernel. ie:

int kernel_size = 3;

REAL kernel[kernel_size * kernel_size] = {
    -1, 0, 1,
    -2, 0, 2,
    -1, 0, 1
}

*/

API
int
ImageKit_Image_ApplyCVKernel(
    ImageKit_Image *self,
    REAL *kernel,
    DIMENSION kernel_size,
    REAL factor,
    REAL bias,
    ImageKit_Coords *coords
);
