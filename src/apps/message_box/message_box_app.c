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
#include "resources.h"

#include <string.h>

typedef struct {
    toml_table_t *startup_config;
    toml_table_t *app_config;
    toml_table_t *messages;
    int selected_message;
} MESSAGE_BOX_CONTEXT;

typedef enum {
    MESSAGE_BOX_GET_MESSAGES,
    MESSAGE_BOX_DISPLAY
} MESSAGE_BOX_STATE;

typedef char* (substitution_value)(MESSAGE_BOX_CONTEXT*);

typedef struct {
    char * to_replace;
    substitution_value *replacement;
} TEXT_SUBSTITUTION;

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

static char* _get_message_author(MESSAGE_BOX_CONTEXT *ctx) {

    int idx = ctx->selected_message;
    toml_array_t *array = toml_array_in(ctx->messages, "message");
    if (!array) {
        return "";
    }

    toml_table_t *message = toml_table_at(array, idx);
    if (!message) {
        return "";
    }

    toml_datum_t author = toml_string_in(message, "author");
    if (!author.ok) {
        return "";
    }

    return author.u.s;
}

static char* _get_message_content(MESSAGE_BOX_CONTEXT *ctx) {
    int idx = ctx->selected_message;
    toml_array_t *array = toml_array_in(ctx->messages, "message");
    if (!array) {
        return "";
    }

    toml_table_t *message = toml_table_at(array, idx);
    if (!message) {
        return "";
    }

    toml_datum_t text = toml_string_in(message, "message_text");
    if (!text.ok) {
        return "";
    }

    return text.u.s;
}

TEXT_SUBSTITUTION _substitutions[2] = {
        {"!!MSG_BOX_AUTHOR!!", _get_message_author},
        {"!!MSG_BOX_MESSAGE!!", _get_message_content}
};


void* message_box_init(toml_table_t *startup_config, toml_table_t * device_config) {

    MESSAGE_BOX_CONTEXT *context = pvPortMalloc(sizeof(MESSAGE_BOX_CONTEXT));

    context->startup_config = startup_config;
    context->app_config = device_config;
    context->messages = resource_get("messages");
    context->selected_message = -1;

    return context;
}

void draw_texts(MESSAGE_BOX_CONTEXT *ctx, toml_table_t* box, DISPLAY_COORD offset, DISPLAY_COORD dims) {

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

        // sensible default
        const FONT_TABLE *font_table = font_get_table_for_name("BITTER_PRO_24");

        if (font.ok) {
            font_table = font_get_table_for_name(font.u.s);
            if (!font_table) {
                font_table = resource_get(font.u.s);
            }
            if (font_table == NULL) {
                printf("Couldn't find font: %s", font.u.s);
            }
        }

        char *substituted_text = NULL;
        toml_datum_t value = toml_string_in(text, "value");
        if (value.ok) {
            render_string = value.u.s;
        }

        for (int j=0; j<sizeof(_substitutions)/sizeof(TEXT_SUBSTITUTION); j++) {
            // Todo - multiple substitutions? Probably an uncommon use case
            char * to_replace = _substitutions[j].to_replace;
            char * match_location = strstr(render_string, to_replace);
            if (match_location) {
                char * replacement = _substitutions[j].replacement(ctx);
                size_t replacement_size = strlen(render_string) - strlen(to_replace) + strlen(replacement) + 1;

                char * replacement_str = pvPortMalloc(replacement_size);
                if (match_location > render_string) {
                    memcpy(replacement_str, render_string, (match_location-render_string));
                }
                replacement_str[match_location-render_string] = '\0';
                strcat(replacement_str, replacement);
                strcat(replacement_str, &render_string[match_location-render_string+strlen(to_replace)]);

                if (substituted_text) {
                    vPortFree(substituted_text);
                }
                substituted_text = replacement_str;
                render_string = replacement_str;
            }
        }

        dispbuf_draw_text(origin, render_string, font_table, bounds.x, bounds.y, flags);
        if (substituted_text) {
            vPortFree(substituted_text);
        }

    }
}

void draw_symbols(MESSAGE_BOX_CONTEXT *ctx, toml_table_t *box, DISPLAY_COORD offset, DISPLAY_COORD dims) {

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

        const FONT_TABLE * font_to_use = font_get_table(SYSTEM_SYMBOLS);

        toml_datum_t font = toml_string_in(symbol, "font");
        if (font.ok) {
            font_to_use = font_get_table_for_name(font.u.s);
            if (!font_to_use) {
                font_to_use = resource_get(font.u.s);
            }
            if (font_to_use == NULL) {
                printf("Couldn't find font: %s", font.u.s);
                font_to_use = font_get_table(SYSTEM_SYMBOLS);
            }
        }

        flags = _parse_draw_flags(symbol);

        const FONT_CHARACTER *c = font_get_bitmap(font_to_use, symbol_id);
        if (c) {
            DISPLAY_COORD symbol_size = {.x=c->width, .y=c->height};
            dispbuf_draw_bitmap(origin, symbol_size, bounds, c->data, flags);
        }
    }
}

void draw_subsections(MESSAGE_BOX_CONTEXT *ctx, toml_table_t *box, DISPLAY_COORD offset, DISPLAY_COORD dims) {

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

        void draw_section(MESSAGE_BOX_CONTEXT *ctx, toml_table_t* box, DISPLAY_COORD offset, DISPLAY_COORD dims);
        draw_section(ctx, sub_box, sub_offset, sub_dims);
    }
}

void draw_rects(MESSAGE_BOX_CONTEXT *ctx, toml_table_t* box, DISPLAY_COORD offset, DISPLAY_COORD dims) {

    toml_array_t *rects = toml_array_in(box, "rect");

    if (!rects) {
        return;
    }

    int num_rects = toml_array_nelem(rects);

    for (int i=0; i<num_rects; i++) {
        toml_table_t *rect = toml_table_at(rects, i);
        if (!rect) {
            continue;
        }
        toml_datum_t x_offset = toml_int_in(rect, "x");
        toml_datum_t y_offset = toml_int_in(rect, "y");
        toml_datum_t width = toml_int_in(rect, "width");
        toml_datum_t height = toml_int_in(rect, "height");
        toml_datum_t thickness = toml_int_in(rect, "thickness");
        toml_datum_t fill = toml_bool_in(rect, "filled");
        toml_datum_t notched = toml_bool_in(rect, "notched");

        DISPLAY_COORD rect_offset = offset;
        if (x_offset.ok) {
            rect_offset.x += x_offset.u.i;
        }
        if (y_offset.ok) {
            rect_offset.y += y_offset.u.i;
        }

        if (!thickness.ok) {
            thickness.u.i = 1;
        }
        if (!width.ok) {
            width.u.i = thickness.u.i;
        }
        if (!height.ok) {
            height.u.i = thickness.u.i;
        }
        if (!fill.ok) {
            fill.u.b = 0;
        }
        if (!notched.ok) {
            notched.u.b = 0;
        }

        DISPLAY_COORD bottom_right = {
            .x = rect_offset.x + width.u.i - 1,
            .y = rect_offset.y + height.u.i - 1
        };
        dispbuf_draw_rect(rect_offset, bottom_right, thickness.u.i, fill.u.b, notched.u.b);
    }
}

void draw_section(MESSAGE_BOX_CONTEXT *ctx, toml_table_t* box, DISPLAY_COORD offset, DISPLAY_COORD dims) {
    draw_texts(ctx, box, offset, dims);
    draw_symbols(ctx, box, offset, dims);
    draw_rects(ctx, box, offset, dims);
    draw_subsections(ctx, box, offset, dims);
}

void message_box_process(void* context, uint8_t *message, size_t length) {

    MESSAGE_BOX_CONTEXT *appCtx = context;

    printf("Message box process...\n");

    appCtx->selected_message++;
    toml_array_t* message_array = toml_array_in(appCtx->messages, "message");
    if (appCtx->selected_message >= toml_array_nelem(message_array)) {
        appCtx->selected_message = 0;
    }

    printf("Displaying message %d\n", appCtx->selected_message);

    dispbuf_swap();
    dispbuf_clear_active();

    toml_table_t *drawing_root = toml_table_in(appCtx->app_config, "render_root");
    DISPLAY_COORD offset = {0, 0};
    DISPLAY_COORD dims = {DISPLAY_WIDTH, DISPLAY_HEIGHT};
    draw_section(appCtx, drawing_root, offset, dims);

    epaper_render_buffer(dispbuf_active_buffer(), dispbuf_inactive_buffer(), BUFFER_SIZE);
}

void message_box_deinit(void* context) {
    vPortFree(context);
}

APP_INTERFACE g_message_box_interface = {
    .name = "message_box",
    .app_init = message_box_init,
    .app_process = message_box_process,
    .app_deinit = message_box_deinit,
    .refresh_rate_ms = 10000
};