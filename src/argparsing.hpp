#pragma once

#include <cstdint>
#include <string_view>

struct Parameters {
    std::string_view output_path;
    bool need_print = false;
    int32_t stats = 10;
    int32_t window = 0;
    int64_t from_time = 0;
    int64_t to_time = 0;

    std::string_view logs_filename;

    bool need_help = false;
};

Parameters ParseArguments(int argc, char** argv);

const char* GetParameterInfo(std::string_view parameter);

void ShowHelpMessage();
