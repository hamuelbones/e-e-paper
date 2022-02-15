//
// Created by Samuel Jones on 12/30/21.
//

#include "display_draw_image.h"
#include "display_draw_geometry.h"

void dispbuf_draw_bitmap(DISPLAY_COORD cursor,
                        DISPLAY_COORD bitmap_size,
                        DISPLAY_COORD bitmap_bounds,
                        const uint8_t *bitmap,
                        DRAW_FLAGS flags) {

    // Upper left corner of image
    DISPLAY_COORD image_base = cursor;

    if (flags & DRAW_JUSTIFY_HORIZ_RIGHT) {
        image_base.x = cursor.x + bitmap_bounds.x - bitmap_size.x;
    } else if (flags & DRAW_JUSTIFY_HORIZ_CENTER) {
        image_base.x = cursor.x + (bitmap_bounds.x - bitmap_size.x)/2;
    }

    if (flags & DRAW_JUSTIFY_VERT_BOTTOM) {
        image_base.y = cursor.y + bitmap_bounds.y - bitmap_size.y;
    } else if (flags & DRAW_JUSTIFY_VERT_CENTER) {
        image_base.y = cursor.y + (bitmap_bounds.y - bitmap_size.y)/2;
    }

    // draw image pixels
    for (int j=0; j<bitmap_size.y; j++) {
        int image_y = j + image_base.y;
        // Skip pixels that aren't in the render area
        if (image_y < cursor.y || image_y >= cursor.y + bitmap_bounds.y) {
            continue;
        }

        int byte_offset = j * ((bitmap_size.x + 7) / 8);
        for (int i=0; i<bitmap_size.x; i++) {
            int image_x = i + image_base.x;
            // Skip pixels that aren't in the render area
            if (image_x < cursor.x || image_x >= cursor.x + bitmap_bounds.x) {
                continue;
            }

            if (bitmap[byte_offset+i/8] & (1<<(7-i%8))) {
                dispbuf_draw_point(image_x, image_y);
            } else {
                dispbuf_clear_point(image_x, image_y);
            }
        }
    }

#if 0
    for(int j=cursor.y; j<cursor.y+bitmap_size.y; j++) {
        int byte_offset = ((bitmap_size.x + 7) / 8)*(j-cursor.y);
        for (int i=cursor.x; i<cursor.x+bitmap_size.x; i++) {
            if (bitmap[byte_offset+(i-cursor.x)/8] & (1<<(7-(i-cursor.x)%8))) {
                dispbuf_draw_point(i, j);
            } else {
                dispbuf_clear_point(i, j);
            }
        }
    }
#endif
}