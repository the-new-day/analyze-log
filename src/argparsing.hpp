#pragma once

#include <cstdint>
#include <expected>
#include <string_view>

struct ParametersParseError {
    const char* message = nullptr;
    const char* argument = nullptr;
};

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

ParametersParseError MakeParametersParseError(const char* message, const char* argument = nullptr);

std::expected<Parameters, ParametersParseError> ParseArguments(int argc, char** argv);

std::expected<const char*, const char*> GetParameterInfo(std::string_view parameter);

void ShowHelpMessage();

std::expected<int64_t, const char*> ParseInt(std::string_view str);
