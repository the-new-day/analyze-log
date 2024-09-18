#pragma once

#include <cstdint>

struct Parameters {
    char* output_path = nullptr;
    bool need_print = false;
    uint32_t stats = 10;
    uint32_t window = 0;
    uint32_t from_time = 0;
    uint32_t to_time = 0;

    char* logs_filename = nullptr;
};

Parameters ParseArgs(int argc, char** argv);
