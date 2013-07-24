
/*

TODO List

- Attach ImageKit_Error pointer to ImageKit_Image objects?
    - Might make multi-threaded algorithms easier.
- Test SavePNG() with 16-bit color depth on Big-Endian
- Support for 12-bit JPEG
- Update documentation for thread-safe error handling.
- ImageKit_Cleanup() function, to cleanup memory (esp. error handling).
- B-Spline point function filter
    - Generate lookup table based on output
        - X coordinate is the input (key)
        - Y coordinate is the output (value)
    - Apply point function using lookup table
- Support for common video frame formats (YCbCr)
- GIF support
- Get box?
- JPEG Grayscale support
- Non-RGB colorspaces for JPEG
- Ability to set bit depth in SavePNG()?
- Better and more complete tests
- Matrix
- Convolution kernel
- Rank filter
- Bilateral filter
- Resize, nearest, bilinear, cubic family (B-Spline, Mitchell and Catmull-Rom), Lanczos3
- Rotate

*/
