//
// Created by Samuel Jones on 1/22/22.
//

#include "file_utils.h"

bool file_copy(const char* to, const char* from) {

    file_handle in_f = FS_Open(from, "r");
    if (!in_f) {
        return false;
    }

    file_handle out_f = FS_Open(to, "w");
    if (!out_f) {
        FS_Close(in_f);
        return false;
    }

    while (!FS_Feof(in_f)) {
        char read_chunk[64];
        int read_len = FS_Read(in_f, read_chunk, 64);
        if (read_len > 0) {
            FS_Write(out_f, read_chunk, read_len);
        }
    }

    FS_Close(in_f);
    FS_Close(out_f);

    struct stat in_stat = {0};
    struct stat out_stat = {0};

    FS_Stat(from, &in_stat);
    FS_Stat(to, &out_stat);

    // Files same size - assume file copy was successful.
    return (in_stat.st_size == out_stat.st_size);
}


bool file_exists(const char* path) {
    struct stat s;
    return (0 == FS_Stat(path, &s));
}