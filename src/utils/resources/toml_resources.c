//
// Created by Samuel Jones on 1/22/22.
//

#include "toml_resources.h"
#include "filesystem_hal.h"
#include "toml.h"
#include "freertos/FreeRTOS.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include "cryptography_hal.h"

#define MAX_TOKEN_SIZE (30)
#define MAX_VARIABLES  (10)

void * toml_load(const char* file_name) {

    file_handle f = fs_open(file_name, "r");
    if (!f) {
        return NULL;
    }

    char toml_error_msg[100];
    toml_table_t *table = toml_parse_file(f, toml_error_msg, 100);
    fs_close(f);

    if (!table) {
        return NULL;
    }

    TOML_RESOURCE_CONTEXT *toml_res_ctx = pvPortMalloc(sizeof(TOML_RESOURCE_CONTEXT));
    toml_res_ctx->variable_count = 0;
    toml_res_ctx->document = table;
    toml_res_ctx->variables = pvPortMalloc(MAX_VARIABLES * sizeof(TOML_RESOURCE_VARIABLE));
    memset(toml_res_ctx->variables, 0, MAX_VARIABLES* sizeof(TOML_RESOURCE_VARIABLE));
    toml_res_ctx->frame_count = 0;
    return (void*) toml_res_ctx;
}

void toml_unload(void* table) {
    TOML_RESOURCE_CONTEXT *toml_res_ctx = table;
    toml_free((toml_table_t *)toml_res_ctx->document);
    vPortFree(toml_res_ctx->variables);
    vPortFree(toml_res_ctx);
}


void toml_new_frame(void* table) {
    TOML_RESOURCE_CONTEXT *toml_res_ctx = table;
    toml_res_ctx->frame_count += 1;
}

static void toml_get_next_token(const char** key, char* token, size_t max_token_size, bool* is_table, bool *is_array, bool* end) {
    *is_array = false;
    *is_table = false;
    *end = false;

    size_t name_len = 0;
    bool has_array_start = false;

    while (**key) {
        // Table access detection
        if (**key == '.') {
            if (name_len == 0) {
                (*key)++;
                continue;
            } else {
                *is_table = true;
                break;
            }
        }
        if ((**key == '[') && (name_len != 0)) {
            *is_table = true;
            break;
        }
        // Array access detection
        if ((**key == '[') && (name_len == 0)) {
            has_array_start = true;
            (*key)++;
            continue;
        }
        if ((**key == ']') && (has_array_start)) {
            *is_array = true;
            (*key)++;
            if ((*key)[1] == '\0') {
                *end = true;
            }
            break;
        }

        token[name_len++] = **key;
        (*key)++;
        if (name_len >= max_token_size) {
            name_len = max_token_size-1;
        }
    }
    token[name_len] = '\0';

    if (!(**key)) {
        *end = true;
    }
}

static bool is_numeric(const char* str) {
    if (!isdigit((unsigned char)*str) && *str!='-' && *str!='+') {
        return false;
    }
    str++;
    while (*str) {
        if (!isdigit((unsigned char)str[0])) {
            return false;
        }
        str++;
    }
    return true;
}

static int random_index(TOML_RESOURCE_CONTEXT* ctx, toml_array_t *array) {
    int nelem = toml_array_nelem(array);
    if (nelem == 0) {
        return -1;
    }
    uint32_t random;
    cryptography_random_bytes((uint8_t*)&random, sizeof(uint32_t));
    return (int) (random % nelem);
}

static int last_index(TOML_RESOURCE_CONTEXT* ctx, toml_array_t *array) {
    int nelem = toml_array_nelem(array);
    if (nelem == 0) {
        return -1;
    }
    return nelem-1;
}

static int incrementing_index(TOML_RESOURCE_CONTEXT* ctx, toml_array_t *array) {
    int nelem = toml_array_nelem(array);
    if (nelem == 0) {
        return -1;
    }
    return ctx->frame_count % nelem;
}

static const char* index_names[] = {
    "random",
    "last",
    "incrementing",
};

static int (*index_methods[])(TOML_RESOURCE_CONTEXT* ctx, toml_array_t *array) = {
    random_index,
    last_index,
    incrementing_index,
};


static int index_from_name(TOML_RESOURCE_CONTEXT* ctx, const char* name, toml_array_t *array) {

    // Check methods
    for (int i=0; i<sizeof(index_names)/sizeof(const char*); i++) {
        if (strcmp(index_names[i], name) == 0) {
            return index_methods[i](ctx, array);
        }
    }

    // now check variables
    for (int i=0; i<ctx->variable_count; i++) {
        if (strcmp(ctx->variables[i].name, name) == 0) {
            return (int) strtol(ctx->variables[i].value, NULL, 10);
        }
    }

    return -1;
}

static int get_index(TOML_RESOURCE_CONTEXT* ctx, toml_array_t *array, const char * index_str) {

    char* variable_name = NULL;
    char* index_name = NULL;
    char* index_copy = pvPortMalloc(strlen(index_str)+1);
    strcpy(index_copy, index_str);

    char* divider = strrchr(index_copy, '=');
    if (divider) {
        *divider = '\0';
        variable_name = index_copy;
        index_name = divider+1;
    } else {
        variable_name = NULL;
        index_name = index_copy;
    }

    int index = -1;
    if (is_numeric(index_name)) {
        index = (int) strtol(index_name, NULL, 10);
    } else {
        index = index_from_name(ctx, index_name, array);
    }

    if (variable_name && index_name) {

        size_t variable_index = 0;
        for (variable_index=0; variable_index < ctx->variable_count; variable_index++) {
            if (strcmp(variable_name, ctx->variables[variable_index].name) == 0) {
                break;
            }
        }
        if (variable_index < MAX_VARIABLES) {

            if (variable_index == ctx->variable_count) {
                // New variable
                ctx->variables[variable_index].name = pvPortMalloc(strlen(variable_name)+1);
                strcpy(ctx->variables[variable_index].name, variable_name);
                ctx->variable_count++;
            }
            // set / update variable value
            if (ctx->variables[variable_index].value) {
                vPortFree(ctx->variables[variable_index].value);
            }
            char* new_value = pvPortMalloc(15);
            snprintf(new_value, 15, "%d", index);
            ctx->variables[variable_index].value = new_value;

        } else {
            printf("Out of variable space, couldn't save: %s", variable_name);
        }
    }

    vPortFree(index_copy);
    return index;
}

void* toml_get_element(void* table, const char *key) {

    TOML_RESOURCE_CONTEXT *toml_res_ctx = table;

    bool currently_parsing_table = true;
    toml_table_t *current_table = toml_res_ctx->document;
    toml_array_t *current_array = NULL;

    while (1) {

        // Get the next thing to interpret, which is:
        // A table name, where we'll run into the `.` separator
        // An array access, where we'll get a string inside square brackets
        // The final object, where we should hit the end of the string.
        char current_name[MAX_TOKEN_SIZE];
        bool is_table = false;
        bool is_array = false;
        bool is_end = false;

        toml_get_next_token(&key, current_name, MAX_TOKEN_SIZE, &is_table, &is_array, &is_end);

        if (is_table) {
            if (!currently_parsing_table) {
                return NULL;
            }
            toml_table_t *probe_table = toml_table_in(current_table, current_name);
            if (probe_table) {
                current_table = probe_table;
                currently_parsing_table = true;
                continue;
            }
            toml_array_t *probe_array = toml_array_in(current_table, current_name);
            if (probe_array) {
                current_array = probe_array;
                currently_parsing_table = false;
                continue;
            }
            return NULL;
        } else if (is_array) {
            if (currently_parsing_table) {
                return NULL;
            }
            int index = get_index(toml_res_ctx, current_array, current_name);

            if (!is_end) {
                toml_table_t *probe_table = toml_table_at(current_array, index);
                if (probe_table) {
                    current_table = probe_table;
                    currently_parsing_table = true;
                    continue;
                }
                toml_array_t *probe_array = toml_array_at(current_array, index);
                if (probe_array) {
                    current_array = probe_array;
                    currently_parsing_table = false;
                    continue;
                }
                return NULL;
            } else { // At the end
                toml_datum_t datum = toml_string_at(current_array, index);
                if (datum.ok) {
                    return datum.u.s;
                }
                datum = toml_int_at(current_array, index);
                if (datum.ok) {
                    return datum.u.s;
                }
                datum = toml_double_at(current_array, index);
                if (datum.ok) {
                    return datum.u.s;
                }
                datum = toml_bool_at(current_array, index);
                if (datum.ok) {
                    return datum.u.s;
                }
                datum = toml_timestamp_at(current_array, index);
                if (datum.ok) {
                    return datum.u.s;
                }
                return NULL;
            }

        } else { // At end, and at a table parsing point
            if (!currently_parsing_table) {
                return NULL;
            }

            toml_datum_t datum;

            datum = toml_string_in(current_table, current_name);
            if (datum.ok) {
                return datum.u.s;
            }
            datum = toml_int_in(current_table, current_name);
            if (datum.ok) {
                return datum.u.s;
            }
            datum = toml_double_in(current_table, current_name);
            if (datum.ok) {
                return datum.u.s;
            }
            datum = toml_bool_in(current_table, current_name);
            if (datum.ok) {
                return datum.u.s;
            }
            datum = toml_timestamp_in(current_table, current_name);
            if (datum.ok) {
                return datum.u.s;
            }
            return NULL;
        }
    }
}