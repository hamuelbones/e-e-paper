//
// Created by Samuel Jones on 1/22/22.
//

#include "file_utils.h"
#include "toml.h"
#include <string.h>

bool file_copy(const char* to, const char* from) {

    printf("Copy from %s to %s\n", from, to);

    file_handle in_f = fs_open(from, "r");
    if (!in_f) {
        printf("No source file \n");
        return false;
    }

    fs_remove(to);
    file_handle out_f = fs_open(to, "wb");
    if (!out_f) {
        printf("Can't open output file \n");
        fs_close(in_f);
        return false;
    }

    while (!fs_feof(in_f)) {
        char read_chunk[64];
        int read_len = fs_read(in_f, read_chunk, 64);
        if (read_len > 0) {
            fs_write(out_f, read_chunk, read_len);
        }
    }

    fs_close(in_f);
    fs_close(out_f);

    struct stat in_stat = {0};
    struct stat out_stat = {0};

    fs_stat(from, &in_stat);
    fs_stat(to, &out_stat);

    printf("File copy: old size: %lld, new size: %lld\n",
           in_stat.st_size, out_stat.st_size);

    // Files same size - assume file copy was successful.
    return (in_stat.st_size == out_stat.st_size);
}


bool file_exists(const char* path) {
    struct stat s;
    return (0 == fs_stat(path, &s));
}

// UUID is a fixed length string
bool file_load_uuid(const char* path, char uuid[37]) {
    void *f = fs_open(path, "r");
    if (!f)  {
        printf("Cannot find expected UUID file!\n");
        return false;
    }

    toml_table_t *root = toml_parse_file(f, NULL, 0);
    fs_close(f);

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
    return fs_read(stream, ptr, size*nitems);
}

void file_print(const char* path) {
    file_handle *f = fs_open(path, "r");
    if (!f) {
        return;
    }
    printf("File contents -- %s --\n\n", path);
    while (!fs_feof(f)) {
        char d[64];
        int read_len = fs_read(f, d, 64);
        for (int i=0; i<read_len; i++) {
            putc(d[i], stdout);
        }
    }
    putc('\n', stdout);
    putc('\n', stdout);
    fs_close(f);
}