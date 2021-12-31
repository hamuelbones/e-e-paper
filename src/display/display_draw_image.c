//
// Created by Samuel Jones on 12/30/21.
//

#include "display_draw_image.h"
#include "display_draw_geometry.h"

void DISPBUF_DrawBitmap(DISPLAY_COORD cursor, uint16_t width, uint16_t height, const uint8_t *bitmap) {
    for(int j=cursor.y; j<cursor.y+height; j++) {
        int offset = ((width + 7) / 8)*(j-cursor.y);
        for (int i=cursor.x; i<cursor.x+width; i++) {
            if (bitmap[offset+(i-cursor.x)/8] & (1<<(7-(i-cursor.x)%8))) {
                DISPBUF_DrawPoint(i, j);
            } else {
                DISPBUF_ClearPoint(i, j);
            }
        }
    }
}