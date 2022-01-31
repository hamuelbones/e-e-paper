//
// Created by Samuel Jones on 1/22/22.
//

#include "file_utils.h"
#include "toml.h"
#include <string.h>

bool file_copy(const char* to, const char* from) {

    printf("Copy from %s to %s\n", from, to);

    file_handle in_f = FS_Open(from, "r");
    if (!in_f) {
        printf("No source file \n");
        return false;
    }

    FS_Remove(to);
    file_handle out_f = FS_Open(to, "wb");
    if (!out_f) {
        printf("Can't open output file \n");
        FS_Close(in_f);
        return false;
    }

    while (!FS_Feof(in_f)) {
        char read_chunk[64];
        int read_len = FS_Read(in_f, read_chunk, 64);
        if (read_len > 0) {
            int write_len = FS_Write(out_f, read_chunk, read_len);
        }
    }

    FS_Close(in_f);
    FS_Close(out_f);

    struct stat in_stat = {0};
    struct stat out_stat = {0};

    FS_Stat(from, &in_stat);
    FS_Stat(to, &out_stat);

    printf("File copy: old size: %d, new size: %d\n",
           in_stat.st_size, out_stat.st_size);

    // Files same size - assume file copy was successful.
    return (in_stat.st_size == out_stat.st_size);
}


bool file_exists(const char* path) {
    struct stat s;
    return (0 == FS_Stat(path, &s));
}

// UUID is a fixed length string
bool file_load_uuid(const char* path, char uuid[37]) {
    void *f = FS_Open(path, "r");
    if (!f)  {
        printf("Cannot find expected UUID file!\n");
        return false;
    }

    toml_table_t *root = toml_parse_file(f, NULL, 0);
    FS_Close(f);

    if (!root) {
        printf("Unable to parse UUID file!\n");
        return false;
    }

    toml_datum_t toml_uuid = toml_string_in(root, "uuid");
    if (!toml_uuid.ok) {
        toml_free(root);
        return false;
    }

    if (strlen(toml_uuid.u.s) >= 37) {
        printf("UUID is too long.");
        toml_free(root);
        return false;
    }

    strcpy(uuid, toml_uuid.u.s);
    toml_free(root);
    return true;
}

size_t toml_fs_read(void* ptr, size_t size, size_t nitems, void* stream) {
    return FS_Read(stream, ptr, size*nitems);
}

void file_print(const char* path) {
    file_handle *f = FS_Open(path, "r");
    if (!f) {
        return;
    }
    printf("File contents -- %s --\n\n", path);
    while (!FS_Feof(f)) {
        char d[64];
        int read_len = FS_Read(f, d, 64);
        for (int i=0; i<read_len; i++) {
            putc(d[i], stdout);
        }
    }
    putc('\n', stdout);
    putc('\n', stdout);
    FS_Close(f);
}