
/*

TODO List

- Cleanup public api polution
- Fill Rect
- Drawing functions:
    - Bresenham line
    - Bresenham circle
    - Bresenham elipse
    - Antialised drawing methods

- Unit Tests for ImageKit_Image_ApplyMatrix()

- Add internal helper functions:
    ImageKit_GetClampRanges(int32_t fmt, REAL scale, REAL *min, REAL *max)
    ImageKit_Image_GetClampRanges(ImageKit_Image *self, REAL *min, REAL *max)

- Add preserve_alpha option to ImageKit_Image_ApplyRankFilter(), like with
    ImageKit_Image_ApplyCVKernel()
- Test Support for 12-bit JPEG in LoadJPEG
- Add support for 12-bit JPEG in SaveJPEG
- Add support for greyscale JPEGs in Load/SaveJPEG
- Non-RGB colorspaces for JPEG?
- Overhaul error system:
    - ClearError()?

- Test SavePNG() with 16-bit color depth on Big-Endian
- Ability to set bit depth in SavePNG()?

- Support for common video frame formats (YCbCr)
- GIF support
- Get box?
- Better and more complete tests
- Bilateral filter
- Resize, cubic family (B-Spline, Mitchell and Catmull-Rom), Lanczos3
- Rotate

*/
