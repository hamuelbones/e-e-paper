//
// Created by Samuel Jones on 2/1/22.
//

#include "font_resources.h"
#include "filesystem_hal.h"
#include <string.h>
#include "freertos/FreeRTOS.h"

static FONT_RESOURCE_CTX _font_resources;

bool font_resource_load(const char* file_name, const char* resource_name) {

    if (_font_resources.count >= MAX_FONT_RESOURCES) {
        return false;
    }

    struct stat s;
    fs_stat(file_name, &s);
    printf("Loading resource: %s, len: %lld\n", file_name, s.st_size);
    file_handle f = fs_open(file_name, "rb");
    if (!f) {
        return false;
    }

    char magic[4] = {0};
    fs_read(f, magic, 4);
    if (strncmp(magic, "FBIN", 4) != 0) {
        printf("Magic marker didn't match\n");
        fs_close(f);
        return false;
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


    size_t res_name_len = strlen(resource_name);

    size_t new_id = _font_resources.count;
    FONT_RESOURCE *new_res = &_font_resources.resource[new_id];

    new_res->name = pvPortMalloc(res_name_len+1);
    strcpy(new_res->name, resource_name);
    new_res->font = table;

    _font_resources.count++;
    fs_close(f);
    return true;
}


FONT_TABLE* font_resource_get(const char* name) {

    for (int i=0; i<_font_resources.count; i++) {
        if (0 == strcmp(_font_resources.resource[i].name, name)) {
            return _font_resources.resource[i].font;
        }
    }
    return NULL;

}

bool font_resource_unload_all() {
    for (int i = 0; i < _font_resources.count; i++) {
        vPortFree(_font_resources.resource[i].name);

        for (int j = 0; j < _font_resources.resource[i].font->code_length; j++) {
            vPortFree((void*)_font_resources.resource[i].font->characters[i]->data);
            vPortFree(&_font_resources.resource[i].font->characters[i]);
        }
        vPortFree(_font_resources.resource[i].font->characters);
        vPortFree(_font_resources.resource[i].font);
    }
    _font_resources.count = 0;
    return true;
}