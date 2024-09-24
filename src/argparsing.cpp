#include "argparsing.hpp"

#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>

constexpr const char* kMissingArgumentMsg{"Unspecified argument value (unexpected end of argument sequence)"};

const char* GetParameterInfo(const char* parameter) {
    if (std::strcmp(parameter, "--stats") == 0 || std::strcmp(parameter, "-s") == 0) {
        return "--stats=<amount> | -s <amount>             [int, >= 0, default=10]       Output first n most frequent requests "
               "finished with code 5XX (in order of frequency)";
    } else if (std::strcmp(parameter, "--window") == 0 || std::strcmp(parameter, "-w") == 0) {
        return "--window=<seconds> | -w <second>           [int, >= 0, default=0]        Output a window of n seconds on which number of "
               "requests was the highest. By default, no such window is calculated";
    } else if (std::strcmp(parameter, "--from") == 0 || std::strcmp(parameter, "-f") == 0) {
        return "--from=<timestamp> | -f <timestamp>        [int, >= 0, default=smallest] Ignore time before the specified";
    } else if (std::strcmp(parameter, "--to") == 0 || std::strcmp(parameter, "-t") == 0) {
        return "--to=<timestamp> | -e <timestamp>          [int, >= 0, default=greatest] Ignore time after the specified";
    } else if (std::strcmp(parameter, "--output") == 0 || std::strcmp(parameter, "-o") == 0) {
        return "--output=<path> | -o <path>                [string]                      Path to the file to which 5XX requests will be written";
    } else if (std::strcmp(parameter, "--print") == 0 || std::strcmp(parameter, "-p") == 0) {
        return "--print | -p                               [flag]                        If specified, 5XX requests will be printed to stdout";
    } else if (std::strcmp(parameter, "--help") == 0 || std::strcmp(parameter, "-h") == 0) {
        return "--help | -h                                [flag]                        Show help and exit";
    } else if (std::strcmp(parameter, "--invalid-lines-output") == 0 || std::strcmp(parameter, "-i") == 0) {
        return "--invalid-lines-output=<path> | -i <path>  [string]                      Path to the file to which "
               "invalid lines from the input file will be written";
    }

    std::stringstream error_message;
    error_message << "Unknown argument: \"" << parameter << '"';

    throw std::invalid_argument(error_message.str());
}

void ShowHelpMessage() {
    std::cout << "Usage: AnalyzeLog [OPTIONS] <logs_filename>\nPossible options:\n\t";
    std::cout << GetParameterInfo("-o");
    std::cout << "\n\t";
    std::cout << GetParameterInfo("-p");
    std::cout << "\n\t";
    std::cout << GetParameterInfo("-s");
    std::cout << "\n\t";
    std::cout << GetParameterInfo("-w");
    std::cout << "\n\t";
    std::cout << GetParameterInfo("-f");
    std::cout << "\n\t";
    std::cout << GetParameterInfo("-t");
    std::cout << "\n\t";
    std::cout << GetParameterInfo("-i");
    std::cout << "\n\t";
    std::cout << GetParameterInfo("-h");
}

int64_t ParseInt(std::string_view str) {
    int64_t result;
    std::from_chars_result convertion_result = std::from_chars(str.data(), str.data() + str.size(), result);

    if (convertion_result.ec == std::errc::invalid_argument || convertion_result.ptr != str.end()) {
        std::stringstream error_message("Cannot parse an integer from \"");
        error_message << std::string(str).c_str() << '"';

        throw std::invalid_argument(error_message.str());
    }

    return result;
}

Parameters ParseArguments(int argc, char** argv) {
    Parameters parameters;

    bool options_ended = false;

    for (int32_t i = 1; i < argc; ++i) {
        char* argument = argv[i];

        if (options_ended || argument[0] != '-') {
            parameters.logs_filename = argument;
            continue;
        }

        if (std::strcmp(argument, "--") == 0) {
            options_ended = true;
            continue;
        }

        if (std::strcmp(argument, "--output") == 0 || std::strcmp(argument, "-o") == 0) {
            if (i == argc - 1) {
                throw std::runtime_error(kMissingArgumentMsg);
            }

            parameters.output_path = argv[++i];
        } else if (std::strncmp(argument, "-o", 2) == 0) {
            parameters.output_path = argument + 2;
        } else if (std::strncmp(argument, "--output=", 9) == 0) {
            parameters.output_path = argument + 9;
        } else if (std::strcmp(argument, "--stats") == 0 || std::strcmp(argument, "-s") == 0) {
            if (i == argc - 1) {
                throw std::runtime_error(kMissingArgumentMsg);
            }

            parameters.stats = ParseInt(argv[++i]);
        } else if (std::strncmp(argument, "-s", 2) == 0) {
            parameters.stats = ParseInt(argument + 2);
        } else if (std::strncmp(argument, "--stats=", 8) == 0) {
            parameters.stats = ParseInt(argument + 8);
        } else if (std::strcmp(argument, "--window") == 0 || std::strcmp(argument, "-w") == 0) {
            if (i == argc - 1) {
                throw std::runtime_error(kMissingArgumentMsg);
            }

            parameters.window = ParseInt(argv[++i]);
        } else if (std::strncmp(argument, "-w", 2) == 0) {
            parameters.window = ParseInt(argument + 2);
        } else if (std::strncmp(argument, "--window=", 9) == 0) {
            parameters.window = ParseInt(argument + 9);
        } else if (std::strcmp(argument, "--from") == 0 || std::strcmp(argument, "-f") == 0) {
            if (i == argc - 1) {
                throw std::runtime_error(kMissingArgumentMsg);
            }

            parameters.from_time = ParseInt(argv[++i]);
        } else if (std::strncmp(argument, "-f", 2) == 0) {
            parameters.from_time = ParseInt(argument + 2);
        } else if (std::strncmp(argument, "--from=", 7) == 0) {
            parameters.from_time = ParseInt(argument + 7);
        } else if (std::strcmp(argument, "--to") == 0 || std::strcmp(argument, "-t") == 0) {
            if (i == argc - 1) {
                throw std::runtime_error(kMissingArgumentMsg);
            }

            parameters.to_time = ParseInt(argv[++i]);
        } else if (std::strncmp(argument, "-t", 2) == 0) {
            parameters.to_time = ParseInt(argument + 2);
        } else if (std::strncmp(argument, "--to=", 5) == 0) {
            parameters.to_time = ParseInt(argument + 5);
        } else if (std::strcmp(argument, "--print") == 0 || std::strcmp(argument, "-p") == 0) {
            parameters.need_print = true;
        } else if (std::strcmp(argument, "--help") == 0 || std::strcmp(argument, "-h") == 0) {
            parameters.need_help = true;
        } else if (std::strcmp(argument, "--invalid-lines-output") == 0 || std::strcmp(argument, "-i") == 0) {
            if (i == argc - 1) {
                throw std::runtime_error(kMissingArgumentMsg);
            }

            parameters.invalid_lines_output_path = argv[++i];
        } else if (std::strncmp(argument, "-i", 2) == 0) {
            parameters.invalid_lines_output_path = argument + 2;
        } else if (std::strncmp(argument, "--invalid-lines-output=", 23) == 0) {
            parameters.invalid_lines_output_path = argument + 23;
        } else {
            std::stringstream error_message("Unknown argument: \"");
            error_message << argument << '"';

            throw std::invalid_argument(error_message.str());
        }
    }
    
    if (parameters.logs_filename == nullptr && !parameters.need_help) {
        throw std::runtime_error("No logs filename is specified");
    } else {
        return parameters;
    }
}
