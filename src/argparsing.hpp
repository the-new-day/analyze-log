#pragma once

#include <cstdint>
#include <string>

struct Parameters {
    std::string output_path;
    bool need_print = false;
    int32_t stats = 10;
    int32_t window = 0;
    int64_t from_time = 0;
    int64_t to_time = 0;

    std::string logs_filename;

    bool need_help = false;
};

Parameters ParseArguments(int argc, char** argv);

std::string GetParameterInfo(const std::string& parameter);

std::string GetHelpMessage();
