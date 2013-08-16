#pragma once

API
int
ImageKit_Image_DrawBresenhamLine(
    ImageKit_Image *self,
    DIMENSION _x0,
    DIMENSION _y0,
    DIMENSION _x1,
    DIMENSION _y1,
    REAL *color
);

API
int
ImageKit_Image_DrawBresenhamCircle(
    ImageKit_Image *self,
    DIMENSION midpoint_x,
    DIMENSION midpoint_y,
    DIMENSION radius,
    REAL *color
);

API
int
ImageKit_Image_DrawBresenhamEllipse(
    ImageKit_Image *self,
    DIMENSION _x0,
    DIMENSION _y0,
    DIMENSION _x1,
    DIMENSION _y1,
    REAL *color
);
