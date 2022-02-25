//
// Created by Samuel Jones on 2/24/22.
//

#include "render_toml.h"
#include "freertos/FreeRTOS.h"
#include "display_draw_geometry.h"
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define INVALID_SIZE (100000)

typedef struct {
    uint16_t top;
    uint16_t bottom;
    uint16_t left;
    uint16_t right;
} BOX_BOUNDS;

typedef enum {
    RENDER_ROOT,
    RENDER_DIV,
    RENDER_RECT,
    RENDER_SYMBOL,
    RENDER_TEXT,
    RENDER_LINE,
    RENDER_TYPE_MAX
} RENDER_OBJECT_TYPE;

const char* render_object_names[RENDER_TYPE_MAX] = {
    "render",
    "div",
    "rect",
    "symbol",
    "text",
    "line"
};


typedef enum {
    LEFT,
    TOP = LEFT,
    CENTER,
    RIGHT,
    BOTTOM = RIGHT,
} POSITION;

typedef struct {
    BOX_BOUNDS margin;
    BOX_BOUNDS padding;
    POSITION vertical_position;
    POSITION horizontal_position;
    POSITION vertical_justification;
    POSITION horizontal_justification;
    int width;
    int height;
} COMMON_RENDER_PROPERTIES;

const COMMON_RENDER_PROPERTIES default_properties = {
    .margin = {0, 0, 0, 0},
    .padding = {0, 0, 0, 0},
    .horizontal_position = LEFT,
    .vertical_position = TOP,
    .horizontal_justification = LEFT,
    .vertical_justification = TOP,
};



static int parse_dimension_string(toml_table_t *item, const char *key, int max_size) {

    toml_datum_t base = toml_int_in(item, key);
    if (base.ok) {
        return (int) base.u.i;
    }

    base = toml_string_in(item, key);
    if (base.ok) {
        bool all_digit = true;
        bool ends_in_percent_sign = false;
        bool ends_in_px = false;
        bool first_char = true;
        const char * iterable_key = key;
        while (*iterable_key) {
            if (!isdigit((int)*iterable_key) &&
                !(first_char && (*iterable_key == '-' || *iterable_key == '+'))) {
                all_digit = false;
                if (*iterable_key == '%') {
                    ends_in_percent_sign = true;
                } else if (!first_char && (*(iterable_key-1) == 'p') && (*iterable_key == 'x')) {
                    ends_in_px  = true;
                } else {
                    ends_in_percent_sign = false;
                    ends_in_px = false;
                }
            } else {
                ends_in_percent_sign = false;
                ends_in_px = false;
            }
            iterable_key++;
            first_char = false;
        }

        if (all_digit) {
            // Just a number (X)
            return strtol(key, NULL, 10);
        } else if (ends_in_px) {
            // A number, with px at the end
            return strtol(key, NULL, 10);
        } else if (ends_in_percent_sign) {
            // XX% or XX.X percent, potentially floating point
            double percentage = strtod(key, NULL);
            return (int)((percentage * max_size) / 100.0f);
        }

        vPortFree(base.u.s);
    }

    return INVALID_SIZE;
}


static void parse_box(toml_table_t *item, const char *key, BOX_BOUNDS *box, DISPLAY_COORD bound_size) {

    // Parse unqualified parameter
    int horizontal = parse_dimension_string(item, key, bound_size.x);
    if (horizontal != INVALID_SIZE) {
        box->left = horizontal;
        box->right = horizontal;
    }
    int vertical = parse_dimension_string(item, key, bound_size.y);
    if (vertical != INVALID_SIZE) {
        box->top = vertical;
        box->bottom = vertical;
    }

    // Parse "horizontal" parameter
    char key_with_suffix[30];
    snprintf(key_with_suffix, 30, "%s%s", key, "_horizontal");
    horizontal = parse_dimension_string(item, key_with_suffix, bound_size.x);
    if (horizontal != INVALID_SIZE) {
        box->left = horizontal;
        box->right = horizontal;
    }

    // Parse "vertical" parameter
    snprintf(key_with_suffix, 30, "%s%s", key, "_vertical");
    vertical = parse_dimension_string(item, key_with_suffix, bound_size.y);
    if (vertical != INVALID_SIZE) {
        box->top = vertical;
        box->bottom = vertical;
    }

    // Parse "top" parameter
    snprintf(key_with_suffix, 30, "%s%s", key, "_top");
    int top = parse_dimension_string(item, key_with_suffix, bound_size.y);
    if (top != INVALID_SIZE) {
        box->top = top;
    };

    // Parse "left" parameter
    snprintf(key_with_suffix, 30, "%s%s", key, "_left");
    int left = parse_dimension_string(item, key_with_suffix, bound_size.x);
    if (left != INVALID_SIZE) {
        box->left = left;
    };

    // Parse "right" parameter
    snprintf(key_with_suffix, 30, "%s%s", key, "_right");
    int right = parse_dimension_string(item, key_with_suffix, bound_size.x);
    if (right != INVALID_SIZE) {
        box->right = right;
    };

    // Parse "bottom" parameter
    snprintf(key_with_suffix, 30, "%s%s", key, "_bottom");
    int bottom = parse_dimension_string(item, key_with_suffix, bound_size.y);
    if (bottom != INVALID_SIZE) {
        box->bottom = bottom;
    };


}

static void parse_orientation(toml_table_t *item, const char* key, POSITION *horizontal, POSITION *vertical) {

    toml_datum_t base = toml_string_in(item, key);
    if (base.ok) {
        if (strcmp(base.u.s, "center") == 0) {
            *horizontal = CENTER;
            *vertical = CENTER;
        } else if (strcmp(base.u.s, "left") == 0) {
            *horizontal = LEFT;
        } else if (strcmp(base.u.s, "right") == 0) {
            *horizontal = RIGHT;
        } else if (strcmp(base.u.s, "top") == 0) {
            *vertical = TOP;
        } else if (strcmp(base.u.s, "bottom") == 0) {
            *vertical = BOTTOM;
        }
        vPortFree(base.u.s);
    }

    char key_with_suffix[30];
    snprintf(key_with_suffix, 30, "%s%s", key, "_horizontal");
    toml_datum_t horizontal_datum = toml_string_in(item, key_with_suffix);
    if (horizontal_datum.ok) {
        if (strcmp(horizontal_datum.u.s, "left") == 0) {
            *horizontal = LEFT;
        } else if (strcmp(horizontal_datum.u.s, "center") == 0) {
            *horizontal = CENTER;
        } else if (strcmp(horizontal_datum.u.s, "right") == 0) {
            *horizontal = RIGHT;
        }
        vPortFree(horizontal_datum.u.s);
    }

    snprintf(key_with_suffix, 30, "%s%s", key, "_vertical");
    toml_datum_t vertical_datum = toml_string_in(item, key_with_suffix);
    if (vertical_datum.ok) {
        if (strcmp(vertical_datum.u.s, "top") == 0) {
            *vertical = TOP;
        } else if (strcmp(vertical_datum.u.s, "center") == 0) {
            *vertical = CENTER;
        } else if (strcmp(vertical_datum.u.s, "bottom") == 0) {
            *vertical = BOTTOM;
        }
        vPortFree(vertical_datum.u.s);
    }
}

void get_render_properties(COMMON_RENDER_PROPERTIES *p, toml_table_t *item, DISPLAY_COORD bound_size) {

    *p = default_properties;

    // Parse margin
    parse_box(item, "margin", &p->margin, bound_size);

    // Parse padding
    parse_box(item, "padding", &p->padding, bound_size);

    // Parse position properties
    parse_orientation(item, "position", &p->horizontal_position, &p->vertical_position);

    // Parse justification properties
    parse_orientation(item, "justify", &p->horizontal_position, &p->vertical_position);


    // Parse width and height
    int width = parse_dimension_string(item, "width", bound_size.x);
    if (width != INVALID_SIZE) {
        p->width = width;
    };
    int height = parse_dimension_string(item, "height", bound_size.y);
    if (height != INVALID_SIZE) {
        p->height = height;
    };

}


static void render_rect(toml_table_t *rect,
                        DISPLAY_COORD offset, DISPLAY_COORD size,
                        POSITION horizontal_justify, POSITION vertical_justify,
                        DISPLAY_COORD *sub_item_offset, DISPLAY_COORD *sub_item_size) {

    toml_datum_t thickness = toml_int_in(rect, "thickness");
    toml_datum_t fill = toml_bool_in(rect, "filled");
    toml_datum_t notched = toml_bool_in(rect, "notched");
    if (!thickness.ok) {
        thickness.u.i = 1;
    }
    if (!fill.ok) {
        fill.u.b = 0;
    }
    if (!notched.ok) {
        notched.u.b = 0;
    }

    DISPLAY_COORD bottom_right = {
        .x = offset.x + size.x,
        .y = offset.y + size.y,
    };

    dispbuf_draw_rect(offset, bottom_right, (int)thickness.u.i, fill.u.b, notched.u.b);

    sub_item_offset->x = offset.x + thickness.u.i;
    sub_item_offset->y = offset.y + thickness.u.i;
    if (sub_item_size->x >= thickness.u.i) {
        sub_item_size->x -= thickness.u.i;
    } else {
        sub_item_size->x = 0;
    }
    if (sub_item_size->y >= thickness.u.i) {
        sub_item_size->y -= thickness.u.i;
    } else {
        sub_item_size->y = 0;
    }

}
static void render_symbol(toml_table_t *symbol,
                          DISPLAY_COORD offset, DISPLAY_COORD size,
                          POSITION horizontal_justify, POSITION vertical_justify,
                          DISPLAY_COORD *sub_item_offset, DISPLAY_COORD *sub_item_size) {

}
static void render_text(toml_table_t *text,
                        DISPLAY_COORD offset, DISPLAY_COORD size,
                        POSITION horizontal_justify, POSITION vertical_justify,
                        DISPLAY_COORD *sub_item_offset, DISPLAY_COORD *sub_item_size) {

}
static void render_line(toml_table_t *line,
                        DISPLAY_COORD offset, DISPLAY_COORD size,
                        POSITION horizontal_justify, POSITION vertical_justify,
                        DISPLAY_COORD *sub_item_offset, DISPLAY_COORD *sub_item_size) {


}

void (*item_draw_functions[RENDER_TYPE_MAX])(toml_table_t *rect, DISPLAY_COORD offset, DISPLAY_COORD size,
                                             POSITION horizontal_justify, POSITION vertical_justify,
                                             DISPLAY_COORD *sub_item_offset, DISPLAY_COORD *sub_item_size) = {
    [RENDER_ROOT] = NULL,
    [RENDER_DIV] = NULL,
    [RENDER_RECT] = render_rect,
    [RENDER_SYMBOL] = render_symbol,
    [RENDER_TEXT] = render_text,
    [RENDER_LINE] = render_line,

};

void render_item(toml_table_t *item, DISPLAY_COORD offset, DISPLAY_COORD dims, RENDER_OBJECT_TYPE type) {

    printf("Render item! %p\n", item);
    if (item == NULL) {
        return;
    }

    COMMON_RENDER_PROPERTIES properties;
    get_render_properties(&properties, item, dims);

    // Can apply margin from now on
    offset.x += properties.margin.left;
    if (properties.margin.left + properties.margin.right > dims.x) {
        printf("Margin too big for area, width forced to 0\n");
        dims.x = 0;
    } else {
        dims.x = dims.x - properties.margin.left - properties.margin.right;
    }

    offset.y += properties.margin.top;
    if (properties.margin.top + properties.margin.bottom > dims.y) {
        printf("Margin too big for area, width forced to 0\n");
        dims.y = 0;
    } else {
        dims.y = dims.y - properties.margin.top - properties.margin.bottom;
    }

    // Now apply position provided we have a known width/height. If not, fill the whole area
    if (!properties.width) {
        properties.width = dims.x;
    }
    if (properties.width > dims.x) {
        printf("Width exceeds margin allowed, trimming\n");
        properties.width = dims.x;
    }
    if (!properties.height) {
        properties.height = dims.y;
    }
    if (properties.height > dims.y) {
        printf("Height exceeds margin allowed, trimming\n");
        properties.height = dims.y;
    }

    DISPLAY_COORD placement;
    if (properties.horizontal_position == LEFT) {
        placement.x = offset.x;
    } else if (properties.horizontal_position == CENTER) {
        placement.x = (2 * offset.x + dims.x - properties.width) / 2;
    } else { // Assumed RIGHT
        placement.x = dims.x + offset.x - properties.width;
    }
    if (properties.vertical_position == TOP) {
        placement.y = offset.y;
    } else if (properties.vertical_position == CENTER) {
        placement.y = (2 * offset.y + dims.y - properties.height) / 2;
    } else { // Assumed BOTTOM
        placement.y = dims.y + offset.y - properties.height;
    }

    // Apply padding and draw

    placement.x += properties.padding.left;
    if (properties.padding.left + properties.padding.right > dims.x) {
        printf("Padding too big for area, width forced to 0\n");
        dims.x = 0;
    } else {
        dims.x = dims.x - properties.padding.left - properties.padding.right;
    }

    placement.y += properties.padding.top;
    if (properties.padding.top + properties.padding.bottom > dims.y) {
        printf("Padding too big for area, width forced to 0\n");
        dims.y = 0;
    } else {
        dims.y = dims.y - properties.padding.top - properties.padding.bottom;
    }

    // Now render and get box for sub-items!
    DISPLAY_COORD sub_item_offset = placement;
    DISPLAY_COORD sub_item_dims = dims;
    if (item_draw_functions[type]) {
        item_draw_functions[type](item, placement, dims,
                properties.horizontal_justification, properties.vertical_justification,
                &sub_item_offset, &sub_item_dims);
    }

    // Check for sub-items!

    for (int i=0; i<RENDER_TYPE_MAX; i++) {
        toml_array_t* arr = toml_array_in(item, render_object_names[i]);
        if (arr) {
            for (int j=0; j<toml_array_nelem(arr); j++) {
                render_item(toml_table_at(arr, j), sub_item_dims, sub_item_offset, i);
            }
        }
    }
}



void render_toml(toml_table_t *render, DISPLAY_COORD dimensions) {
    DISPLAY_COORD offset = {0, 0};
    render_item(render, offset, dimensions, RENDER_ROOT);

}