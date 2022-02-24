//
// Created by Samuel Jones on 2/1/22.
//

#include "font_resources.h"
#include "filesystem_hal.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "toml.h"
#include "fonts.h"

void* font_load(const char* file_name) {

    file_handle f = fs_open(file_name, "rb");
    if (!f) {
        return NULL;
    }

    char magic[4] = {0};
    fs_read(f, magic, 4);
    if (strncmp(magic, "FBIN", 4) != 0) {
        printf("Magic marker didn't match\n");
        fs_close(f);
        return NULL;
    }

    int16_t base = 0, num_characters = 0;
    fs_read(f, &base, 2);
    fs_read(f, &num_characters, 2);
    printf("Importing font - base char: %d, number of chars: %d\n", base, num_characters);\

    FONT_TABLE *table = pvPortMalloc(sizeof(FONT_TABLE));
    table->code_base = base;
    table->code_length = num_characters;
    table->characters = pvPortMalloc(sizeof(FONT_CHARACTER *)*num_characters);

    size_t data_offset = 8 + num_characters * 8;

    for (int i=0; i<num_characters; i++) {
        uint32_t char_base = 8 + i * 8;
        fs_fseek(f, (int)char_base, SEEK_SET);

        table->characters[i] = pvPortMalloc(sizeof(FONT_CHARACTER));
        fs_read(f, (unsigned char*)&table->characters[i]->width, 2);
        fs_read(f, (unsigned char*)&table->characters[i]->height, 2);
        uint32_t data_size = ((table->characters[i]->width + 7) / 8) * table->characters[i]->height;
        uint32_t section_offset = 0;
        fs_read(f, &section_offset, 4);
        fs_fseek(f, (int)(section_offset + data_offset), SEEK_SET);
        table->characters[i]->data = pvPortMalloc(data_size);
        fs_read(f, (void*)table->characters[i]->data, (int)data_size);
    }

    fs_close(f);
    return table;
}


void font_unload(void* table) {

    FONT_TABLE* font_table = (FONT_TABLE*)table;

    for (int j = 0; j < font_table->code_length; j++) {
        vPortFree((void*)font_table->characters[j]->data);
        vPortFree(&font_table->characters[j]);
    }
    vPortFree(font_table->characters);
    vPortFree(font_table);
}

