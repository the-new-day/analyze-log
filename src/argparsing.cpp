#include "argparsing.hpp"

#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstddef>
#include <filesystem>

constexpr const char* kMissingArgumentMsg{"Unspecified argument value (unexpected end of argument sequence)"};

std::expected<const char*, const char*> GetParameterInfo(const char* parameter) {
    if (std::strcmp(parameter, "--stats") == 0 || std::strcmp(parameter, "-s") == 0) {
        return "--stats=<amount> | -s <amount>             [int, >= 0, default=10]       Output first n most frequent requests "
               "finished with code 5XX (in order of frequency)";
    } else if (std::strcmp(parameter, "--window") == 0 || std::strcmp(parameter, "-w") == 0) {
        return "--window=<seconds> | -w <second>           [int, >= 0, default=0]        Output a window of n seconds on which number of "
               "requests was the highest. By default, no such window is calculated";
    } else if (std::strcmp(parameter, "--from") == 0 || std::strcmp(parameter, "-f") == 0) {
        return "--from=<timestamp> | -f <timestamp>        [int, >= 0, default=smallest] Ignore time before the specified";
    } else if (std::strcmp(parameter, "--to") == 0 || std::strcmp(parameter, "-t") == 0) {
        return "--to=<timestamp> | -t <timestamp>          [int, >= 0, default=greatest] Ignore time after the specified";
    } else if (std::strcmp(parameter, "--output") == 0 || std::strcmp(parameter, "-o") == 0) {
        return "--output=<path> | -o <path>                [string, optional]            Path to the file to which 5XX requests will be written. "
               "If not specified, --stats and --print won't work.";
    } else if (std::strcmp(parameter, "--print") == 0 || std::strcmp(parameter, "-p") == 0) {
        return "--print | -p                               [flag, optional]              If specified, 5XX requests will be printed to stdout";
    } else if (std::strcmp(parameter, "--help") == 0 || std::strcmp(parameter, "-h") == 0) {
        return "--help | -h                                [flag, optional]              Show help and exit";
    } else if (std::strcmp(parameter, "--invalid-lines-output") == 0 || std::strcmp(parameter, "-i") == 0) {
        return "--invalid-lines-output=<path> | -i <path>  [string, optional]            Path to the file to which "
               "invalid lines from the input file will be written";
    }

    return std::unexpected{"Cannot get parameter info: unknown parameter"};
}

void ShowHelpMessage() {
    std::cout << "Usage: AnalyzeLog [OPTIONS] <logs_filename>\nPossible options:\n\t";
    std::cout << *GetParameterInfo("-o");
    std::cout << "\n\t";
    std::cout << *GetParameterInfo("-p");
    std::cout << "\n\t";
    std::cout << *GetParameterInfo("-s");
    std::cout << "\n\t";
    std::cout << *GetParameterInfo("-w");
    std::cout << "\n\t";
    std::cout << *GetParameterInfo("-f");
    std::cout << "\n\t";
    std::cout << *GetParameterInfo("-t");
    std::cout << "\n\t";
    std::cout << *GetParameterInfo("-i");
    std::cout << "\n\t";
    std::cout << *GetParameterInfo("-h");
}

std::optional<ParametersParseError> ValidateParameters(const Parameters& parameters) {
    if (parameters.stats < 0 || parameters.window < 0 || parameters.from_time < 0 || parameters.to_time < 0) {
        return MakeParametersParseError("Negative value for a positive integer argument");
    }

    if (parameters.logs_filename != nullptr && !std::filesystem::exists(parameters.logs_filename)) {
        return MakeParametersParseError("Cannot find input file");
    }

    if (parameters.logs_filename == nullptr && !parameters.need_help) {
        return MakeParametersParseError("No logs filename is specified");
    }

    return std::nullopt;
}

ParametersParseError MakeParametersParseError(const char* message, const char* argument) {
    ParametersParseError error;
    error.message = message;
    error.argument = argument;
    return error;
}

std::expected<int64_t, const char*> ParseInt(std::string_view str) {
    int64_t result;
    std::from_chars_result convertion_result = std::from_chars(str.data(), str.data() + str.size(), result);

    if (convertion_result.ec == std::errc::invalid_argument || convertion_result.ptr != str.end()) {
        return std::unexpected{"Cannot parse an integer from non-numeric data"};
    } else if (convertion_result.ec == std::errc::result_out_of_range) {
        return std::unexpected{"The number is too large"};
    }

    return result;
}

bool SetFlag(Parameters& parameters, char* name) {
    if (std::strcmp(name, "--print") == 0 || std::strcmp(name, "-p") == 0) {
        parameters.need_print = true;
        return true;
    } else if (std::strcmp(name, "--help") == 0 || std::strcmp(name, "-h") == 0) {
        parameters.need_help = true;
        return true;
    }

    return false;
}

std::optional<ParametersParseError> ParseOption(Parameters& parameters, char* argument, size_t name_length, char* raw_value) {
    if (std::strncmp(argument, "--output", name_length) == 0 || std::strncmp(argument, "-o", name_length) == 0) {
        parameters.output_path = raw_value;
        return std::nullopt;
    } else if (std::strncmp(argument, "--invalid-lines-output", name_length) == 0 || std::strncmp(argument, "-i", name_length) == 0) {
        parameters.invalid_lines_output_path = raw_value;
        return std::nullopt;
    }

    std::expected<int64_t, const char*> number = ParseInt(raw_value);
    if (!number.has_value()) {
        return MakeParametersParseError(number.error(), argument);
    }
    
    if (std::strncmp(argument, "--stats", name_length) == 0 || std::strncmp(argument, "-s", 2) == 0) {
        parameters.stats = number.value();
    } else if (std::strncmp(argument, "--window", name_length) == 0 || std::strncmp(argument, "-w", 2) == 0) {
        parameters.window = number.value();
    } else if (std::strncmp(argument, "--from", name_length) == 0 || std::strncmp(argument, "-f", 2) == 0) {
        parameters.from_time = number.value();
    } else if (std::strncmp(argument, "--to", name_length) == 0 || std::strncmp(argument, "-t", 2) == 0) {
        parameters.to_time = number.value();
    } else {
        return MakeParametersParseError("Unknown argument", argument);
    }

    return std::nullopt;
}

std::expected<Parameters, ParametersParseError> ParseArguments(int argc, char** argv) {
    Parameters parameters;

    bool options_ended = false;

    for (size_t i = 1; i < argc; ++i) {
        char* argument = argv[i];

        if (options_ended || argument[0] != '-' || std::strlen(argument) == 1) {
            parameters.logs_filename = argument;
            continue;
        }

        if (std::strcmp(argument, "--") == 0) {
            options_ended = true;
            continue;
        }

        if (SetFlag(parameters, argument)) {
            continue;
        }

        char* raw_value = nullptr;
        size_t name_length;
        char* equal_sign = std::strchr(argument, '=');
        
        if (argument[1] != '-' && std::strlen(argument) > 2) {
            // for arguments like "-opath"
            name_length = 2;
            raw_value = argument + 2;
        } else if (equal_sign != nullptr) {
            name_length = equal_sign - argument;
            raw_value = argument + name_length + 1;
        } else if (i != argc - 1) {
            name_length = std::strlen(argument);
            raw_value = argv[++i];
        } else {
            return std::unexpected{MakeParametersParseError(kMissingArgumentMsg)};
        }

        std::optional<ParametersParseError> parsing_result = 
            ParseOption(parameters, argument, name_length, raw_value);

        if (parsing_result.has_value()) {
            return std::unexpected{parsing_result.value()};
        }
    }
    
    std::optional<ParametersParseError> validation_result = ValidateParameters(parameters);

    if (validation_result.has_value()) {
        return std::unexpected{validation_result.value()};
    }

    return parameters;
}
