#pragma once

/* Converts between colorspace formats, but does NOT convert colorspaces! */
API
int
ImageKit_Image_BlitRect(
    ImageKit_Image *dst,
    ImageKit_Rect *dst_rect,
    ImageKit_Image *src,
    ImageKit_Rect *src_rect
);

API
int
ImageKit_Image_BlitCoords(
    ImageKit_Image *dst,
    DIMENSION dst_x,
    DIMENSION dst_y,
    ImageKit_Image *src,
    ImageKit_Coords *src_coords
);
