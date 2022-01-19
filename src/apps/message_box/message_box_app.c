//
// Created by Samuel Jones on 1/8/22.
//

#include "FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "message_box_app.h"
#include "filesystem_hal.h"

#include "epaper_hal.h"
#include "display_buffer.h"
#include "display_draw_image.h"
#include "display_draw_geometry.h"
#include "display_draw_text.h"

#include <string.h>

#define MESSAGES_FILENAME "message_box.toml"

typedef struct {
    toml_table_t *startup_config;
    toml_table_t *app_config;
    toml_table_t *messages;
} MESSAGE_BOX_CONTEXT;

typedef enum {
    MESSAGE_BOX_GET_MESSAGES,
    MESSAGE_BOX_DISPLAY
} MESSAGE_BOX_STATE;

DRAW_FLAGS _parse_draw_flags(toml_table_t *table) {

    DRAW_FLAGS flags = 0;

    toml_datum_t vert_center = toml_bool_in(table, "vertical_center");
    if (vert_center.ok && vert_center.u.b) {
        flags |= DRAW_JUSTIFY_VERT_CENTER;
    }
    toml_datum_t bottom = toml_bool_in(table, "bottom");
    if (bottom.ok && bottom.u.b) {
        flags |= DRAW_JUSTIFY_VERT_BOTTOM;
    }
    toml_datum_t horiz_center = toml_bool_in(table, "horizontal_center");
    if (horiz_center.ok && horiz_center.u.b) {
        flags |= DRAW_JUSTIFY_HORIZ_CENTER;
    }
    toml_datum_t right = toml_bool_in(table, "right");
    if (right.ok && right.u.b) {
        flags |= DRAW_JUSTIFY_HORIZ_RIGHT;
    }

    return flags;
}

void _load_message_file(MESSAGE_BOX_CONTEXT *context) {

    char toml_error_msg[100];
    // Load message file
    file_handle fh = FS_Open(MESSAGES_FILENAME, "r");
    context->messages = toml_parse_file(fh, toml_error_msg, 100);
    if (!context->messages) {
        printf("Toml message file load error: %s\n", toml_error_msg);
    }
    FS_Close(fh);
}

void* message_box_init(toml_table_t *startup_config, toml_table_t * device_config) {

    MESSAGE_BOX_CONTEXT *context = pvPortMalloc(sizeof(MESSAGE_BOX_CONTEXT));

    context->startup_config = startup_config;
    context->app_config = device_config;
    context->messages = NULL;

    // If online, make a request to get messages
    toml_datum_t mode = toml_string_in(startup_config, "mode");
    if (strcmp("online", mode.u.s) == 0) {
        // TODO
    } else {
        _load_message_file(context);
    }

    return context;
}

void draw_texts(toml_table_t* box, DISPLAY_COORD offset, DISPLAY_COORD dims) {

    toml_array_t *texts = toml_array_in(box, "text");
    if (!texts) {
        return;
    }

    int num_texts = toml_array_nelem(texts);

    for (int i=0; i<num_texts; i++) {
        toml_table_t *text = toml_table_at(texts, i);
        if (!text) {
            break;
        }
        DISPLAY_COORD origin = offset;
        DISPLAY_COORD bounds = dims;
        DRAW_FLAGS flags = 0;
        EPAPER_DISPLAY_FONT_ID font_id = BITTER_PRO_10;
        char *render_string = "";

        toml_datum_t x_offset = toml_int_in(text, "x");
        if (x_offset.ok) {
            origin.x += x_offset.u.i;
        }

        toml_datum_t y_offset = toml_int_in(text, "y");
        if (y_offset.ok) {
            origin.y += y_offset.u.i;
        }

        toml_datum_t width = toml_int_in(text, "width");
        if (width.ok) {
            bounds.x = width.u.i;
        }
        toml_datum_t height = toml_int_in(text, "height");
        if (height.ok) {
            bounds.y = height.u.i;
        }

        flags = _parse_draw_flags(text);

        toml_datum_t font = toml_string_in(text, "font");
        if (font.ok) {
            int j = 0;
            while (true) {
                const char *font_name = FONT_GetName(j);
                if (!font_name) {
                    break;
                }
                if (strcmp(font_name, font.u.s) == 0) {
                    font_id = j;
                    break;
                }
                j++;
            }
        }

        toml_datum_t value = toml_string_in(text, "value");
        if (value.ok) {
            render_string = value.u.s;
        }

        DISPBUF_DrawMultiline(origin, render_string, font_id, bounds.x, bounds.y, flags);

    }
}

void draw_symbols(toml_table_t *box, DISPLAY_COORD offset, DISPLAY_COORD dims) {

    toml_array_t *symbols = toml_array_in(box, "symbol");

    if (!symbols) {
        return;
    }

    int num_symbols = toml_array_nelem(symbols);

    for (int i=0; i<num_symbols; i++) {
        toml_table_t *symbol = toml_table_at(symbols, i);
        if (!symbol) {
            break;
        }
        DISPLAY_COORD origin = offset;
        DISPLAY_COORD bounds = {DISPLAY_WIDTH-origin.x, DISPLAY_HEIGHT-origin.y};
        int symbol_id = 0;
        DRAW_FLAGS flags = 0;

        toml_datum_t x_offset = toml_int_in(symbol, "x");
        if (x_offset.ok) {
            origin.x += x_offset.u.i;
        }

        toml_datum_t y_offset = toml_int_in(symbol, "y");
        if (y_offset.ok) {
            origin.y += y_offset.u.i;
        }

        toml_datum_t width = toml_int_in(symbol, "max_width");
        if (width.ok) {
            bounds.x = width.u.i;
        }
        toml_datum_t height = toml_int_in(symbol, "max_height");
        if (height.ok) {
            bounds.y = height.u.i;
        }

        toml_datum_t id = toml_int_in(symbol, "id");
        if (id.ok) {
            symbol_id = id.u.i;
        }

        flags = _parse_draw_flags(symbol);

        const FONT_CHARACTER *c = FONT_GetBitmap(SYSTEM_SYMBOLS, symbol_id);
        if (c) {
            DISPLAY_COORD symbol_size = {.x=c->width, .y=c->height};
            DISPBUF_DrawBitmap(origin, symbol_size, bounds, c->data, flags);
        }
    }
}

void draw_subsections(toml_table_t *box, DISPLAY_COORD offset, DISPLAY_COORD dims) {

    toml_array_t * sub_boxes = toml_array_in(box, "box");

    if (!sub_boxes) {
        return;
    }

    int num_boxes = toml_array_nelem(sub_boxes);

    for (int i=0; i<num_boxes; i++) {
        toml_table_t *sub_box = toml_table_at(sub_boxes, i);
        if (!sub_box) {
            continue;
        }
        toml_datum_t x_offset = toml_int_in(sub_box, "x");
        toml_datum_t y_offset = toml_int_in(sub_box, "y");
        toml_datum_t width = toml_int_in(sub_box, "width");
        toml_datum_t height = toml_int_in(sub_box, "height");

        DISPLAY_COORD sub_offset = offset;
        DISPLAY_COORD sub_dims = dims;
        if (x_offset.ok) {
            sub_offset.x += x_offset.u.i;
        }
        if (y_offset.ok) {
            sub_offset.y += y_offset.u.i;
        }
        if (width.ok) {
            sub_dims.x = width.u.i;
        }
        if (height.ok) {
            sub_dims.y = width.u.i;
        }

        void draw_section(toml_table_t* box, DISPLAY_COORD offset, DISPLAY_COORD dims);
        draw_section(sub_box, sub_offset, sub_dims);
    }
}

void draw_section(toml_table_t* box, DISPLAY_COORD offset, DISPLAY_COORD dims) {
    draw_texts(box, offset, dims);
    draw_symbols(box, offset, dims);
    draw_subsections(box, offset, dims);
}

void message_box_process(void* context, uint8_t *message, size_t length) {

    MESSAGE_BOX_CONTEXT *appCtx = context;

    printf("Message box process...\n");

    DISPBUF_Swap();
    DISPBUF_ClearActive();

    toml_table_t *drawing_root = toml_table_in(appCtx->app_config, "render_root");
    DISPLAY_COORD offset = {0, 0};
    DISPLAY_COORD dims = {DISPLAY_WIDTH, DISPLAY_HEIGHT};
    draw_section(drawing_root, offset, dims);

    EPAPER_RenderBuffer(DISPBUF_ActiveBuffer(), DISPBUF_InactiveBuffer(), BUFFER_SIZE);
}

void message_box_deinit(void* context) {
    vPortFree(context);
}

APP_INTERFACE g_message_box_interface = {
    .name = "message_box",
    .app_init = message_box_init,
    .app_process = message_box_process,
    .app_deinit = message_box_deinit,
    .refresh_rate_ms = 1000
};