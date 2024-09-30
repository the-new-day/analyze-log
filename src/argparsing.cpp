#include "argparsing.hpp"

#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstddef>
#include <filesystem>

const char* kStatsShortArg = "-s";
const char* kStatsLongArg = "--stats";
const char* kWindowShortArg = "-w";
const char* kWindowLongArg = "--window";
const char* kOutputShortArg = "-o";
const char* kOutputLongArg = "--output";
const char* kPrintShortArg = "-p";
const char* kPrintLongArg = "--print";
const char* kFromShortArg = "-f";
const char* kFromLongArg = "--from";
const char* kToShortArg = "-t";
const char* kToLongArg = "--to";
const char* kInvalidLinesShortArg = "-i";
const char* kInvalidLinesLongArg = "--invalid-lines-output";
const char* kHelpShortArg = "-h";
const char* kHelpLongArg = "--help";

const char* kMissingArgumentMsg{"Unspecified argument value (unexpected end of argument sequence)"};

std::expected<const char*, const char*> GetParameterInfo(std::string_view parameter) {
    if (parameter == kStatsLongArg || parameter == kStatsShortArg) {
        return "--stats=<amount> | -s <amount>             [int, >= 0, default=10]       Output first n most frequent requests "
               "finished with code 5XX (in order of frequency)";
    } else if (parameter == kWindowLongArg || parameter == kWindowShortArg) {
        return "--window=<seconds> | -w <second>           [int, >= 0, default=0]        Output a window of n seconds on which number of "
               "requests was the highest. By default, no such window is calculated";
    } else if (parameter == kFromLongArg || parameter == kFromShortArg) {
        return "--from=<timestamp> | -f <timestamp>        [int, >= 0, default=smallest] Ignore time before the specified";
    } else if (parameter == kToLongArg || parameter == kToShortArg) {
        return "--to=<timestamp> | -t <timestamp>          [int, >= 0, default=greatest] Ignore time after the specified";
    } else if (parameter == kOutputLongArg || parameter == kOutputShortArg) {
        return "--output=<path> | -o <path>                [string, optional]            Path to the file to which 5XX requests will be written. "
               "If not specified, --stats and --print won't work.";
    } else if (parameter == kPrintLongArg || parameter == kPrintShortArg) {
        return "--print | -p                               [flag, optional]              If specified, 5XX requests will be printed to stdout";
    } else if (parameter == kHelpLongArg || parameter == kHelpShortArg) {
        return "--help | -h                                [flag, optional]              Show help and exit";
    } else if (parameter == kInvalidLinesLongArg || parameter == kInvalidLinesShortArg) {
        return "--invalid-lines-output=<path> | -i <path>  [string, optional]            Path to the file to which "
               "invalid lines from the input file will be written";
    }

    return std::unexpected{"Cannot get parameter info: unknown parameter"};
}

void ShowHelpMessage() {
    std::cout << *GetParameterInfo(kInvalidLinesLongArg) << '\n';
    std::cout << "Usage: AnalyzeLog [OPTIONS] <logs_filename>" << std::endl << 
        "Possible options:" << std::endl << "\t";
    std::cout << *GetParameterInfo(kOutputLongArg) << std::endl << '\t';
    std::cout << *GetParameterInfo(kPrintLongArg) << std::endl << '\t';
    std::cout << *GetParameterInfo(kStatsLongArg) << std::endl << '\t';
    std::cout << *GetParameterInfo(kWindowLongArg) << std::endl << '\t';
    std::cout << *GetParameterInfo(kFromLongArg) << std::endl << '\t';
    std::cout << *GetParameterInfo(kToLongArg) << std::endl << '\t';
    std::cout << *GetParameterInfo(kInvalidLinesLongArg) << std::endl << '\t';
    std::cout << *GetParameterInfo(kHelpLongArg) << std::endl << '\t';
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
    if (std::strcmp(name, kPrintLongArg) == 0 || std::strcmp(name, kPrintShortArg) == 0) {
        parameters.need_print = true;
        return true;
    } else if (std::strcmp(name, kHelpLongArg) == 0 || std::strcmp(name, kHelpShortArg) == 0) {
        parameters.need_help = true;
        return true;
    }

    return false;
}

std::optional<ParametersParseError> ParseOption(Parameters& parameters, char* argument, size_t name_length, char* raw_value) {
    if (std::strncmp(argument, kOutputLongArg, name_length) == 0 || std::strncmp(argument, kOutputShortArg, name_length) == 0) {
        parameters.output_path = raw_value;
        return std::nullopt;
    } else if (std::strncmp(argument, kInvalidLinesLongArg, name_length) == 0 || std::strncmp(argument, kInvalidLinesShortArg, name_length) == 0) {
        parameters.invalid_lines_output_path = raw_value;
        return std::nullopt;
    }

    std::expected<int64_t, const char*> number = ParseInt(raw_value);
    
    if (std::strncmp(argument, kStatsLongArg, name_length) == 0 || std::strncmp(argument, kStatsShortArg, 2) == 0) {
        if (!number.has_value()) return MakeParametersParseError(number.error(), argument);
        parameters.stats = number.value();
    } else if (std::strncmp(argument, kWindowLongArg, name_length) == 0 || std::strncmp(argument, kWindowShortArg, 2) == 0) {
        if (!number.has_value()) return MakeParametersParseError(number.error(), argument);
        parameters.window = number.value();
    } else if (std::strncmp(argument, kFromLongArg, name_length) == 0 || std::strncmp(argument, kFromShortArg, 2) == 0) {
        if (!number.has_value()) return MakeParametersParseError(number.error(), argument);
        parameters.from_time = number.value();
    } else if (std::strncmp(argument, kToLongArg, name_length) == 0 || std::strncmp(argument, kToShortArg, 2) == 0) {
        if (!number.has_value()) return MakeParametersParseError(number.error(), argument);
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
