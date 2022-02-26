//
// Created by Samuel Jones on 1/22/22.
//

#include "toml_resources.h"
#include "filesystem_hal.h"
#include "toml.h"
#include "freertos/FreeRTOS.h"
#include <string.h>
#include <stdbool.h>

#define MAX_TOKEN_SIZE (30)

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

    return (void*) table;
}

void toml_unload(void* table) {
    toml_free((toml_table_t *)table);
}


void toml_new_frame(void* table) {

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

void* toml_get_element(void* table, const char *key) {

    bool currently_parsing_table = true;
    toml_table_t *current_table = table;
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
            int index = 0;

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
                toml_datum_t *datum = pvPortMalloc(sizeof(toml_datum_t));

                *datum = toml_string_at(current_array, index);
                if (datum->ok) {
                    return datum;
                }
                *datum = toml_int_at(current_array, index);
                if (datum->ok) {
                    return datum;
                }
                *datum = toml_double_at(current_array, index);
                if (datum->ok) {
                    return datum;
                }
                *datum = toml_bool_at(current_array, index);
                if (datum->ok) {
                    return datum;
                }
                *datum = toml_timestamp_at(current_array, index);
                if (datum->ok) {
                    return datum;
                }
                vPortFree(datum);
                return NULL;
            }

        } else { // At end, and at a table parsing point
            if (!currently_parsing_table) {
                return NULL;
            }

            toml_datum_t *datum = pvPortMalloc(sizeof(toml_datum_t));

            *datum = toml_string_in(current_table, current_name);
            if (datum->ok) {
                return datum;
            }
            *datum = toml_int_in(current_table, current_name);
            if (datum->ok) {
                return datum;
            }
            *datum = toml_double_in(current_table, current_name);
            if (datum->ok) {
                return datum;
            }
            *datum = toml_bool_in(current_table, current_name);
            if (datum->ok) {
                return datum;
            }
            *datum = toml_timestamp_in(current_table, current_name);
            if (datum->ok) {
                return datum;
            }
            vPortFree(datum);
            return  NULL;
        }
    }
    return NULL;
}