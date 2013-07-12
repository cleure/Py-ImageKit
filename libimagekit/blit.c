
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include "imagekit.h"

API
int
ImageKit_Image_BlitRect(
    ImageKit_Image *dst,
    ImageKit_Rect *dst_rect,
    ImageKit_Image *src,
    ImageKit_Rect *src_rect
)
{
    DIMENSION src_coords[4];
    DIMENSION dst_coords[4];
    DIMENSION w, h;
    DIMENSION channels, c;
    REAL scales[4];
    
    if (dst->colorspace != src->colorspace) {
        ImageKit_SetError(ImageKit_TypeError, "Colorspaces must be the same");
        return -1;
    }
    
    channels = (dst->channels < src->channels) ? dst->channels : src->channels;
    w = (dst_rect->w < src_rect->w) ? dst_rect->w : src_rect->w;
    h = (dst_rect->h < src_rect->h) ? dst_rect->h : src_rect->h;
    
    src_coords[1] = src_rect->y;
    src_coords[2] = src_rect->x + w;
    src_coords[3] = src_rect->y + h;
    
    dst_coords[1] = dst_rect->y;
    dst_coords[2] = dst_rect->x + w;
    dst_coords[3] = dst_rect->y + h;
    
    if (src->colorspace_format == dst->colorspace_format) {
        ImageKit_Image_GetConversionScales(dst, src, (REAL *)&scales);
        
        while (src_coords[1] < src_coords[3]) {
            src_coords[0] = src_rect->x;
            dst_coords[0] = dst_rect->x;
        
            while (src_coords[0] < src_coords[2]) {
            
                for (c = 0; c < channels; c++) {
                    dst->data[PIXEL_INDEX(dst, dst_coords[0], dst_coords[1]) + c] =
                        src->data[PIXEL_INDEX(src, src_coords[0], src_coords[1]) + c];
                }
            
                src_coords[0]++;
                dst_coords[0]++;
            }
        
            src_coords[1]++;
            dst_coords[1]++;
        }
    } else {
        while (src_coords[1] < src_coords[3]) {
            src_coords[0] = src_rect->x;
            dst_coords[0] = dst_rect->x;
        
            while (src_coords[0] < src_coords[2]) {
            
                for (c = 0; c < channels; c++) {
                    dst->data[PIXEL_INDEX(dst, dst_coords[0], dst_coords[1]) + c] =
                        src->data[PIXEL_INDEX(src, src_coords[0], src_coords[1]) + c] * scales[c];
                }
            
                src_coords[0]++;
                dst_coords[0]++;
            }
        
            src_coords[1]++;
            dst_coords[1]++;
        }
    }
    
    return 0;
}
