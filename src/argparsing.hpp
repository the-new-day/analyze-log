#pragma once

#include <cstdint>
#include <string_view>

struct Parameters {
    char* output_path = nullptr;
    bool need_print = false;
    int32_t stats = 10;
    int32_t window = 0;
    int64_t from_time = 0;
    int64_t to_time = 0;

    char* logs_filename = nullptr;

    bool need_help = false;

    char* invalid_lines_output_path = nullptr;
};

Parameters ParseArguments(int argc, char** argv);

const char* GetParameterInfo(const char* parameter);

void ShowHelpMessage();
