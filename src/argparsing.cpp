#include "argparsing.hpp"

#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <iostream>

const std::string kMissingArgumentMsg{"Unspecified argument value (unexpected end of argument sequence)"};

std::string GetParameterInfo(const std::string& parameter) {

    if (parameter == "--stats" || parameter == "-s") {
        return "--stats=<amount> | -s <amount>      [int, >= 0, default=10]       Output first n most frequent requests "
               "finished with code 5XX (in order of frequency)";
    } else if (parameter == "--window" || parameter == "-w") {
        return "--window=<seconds> | -w <second>    [int, >= 0, default=0]        Output a window of n seconds on which number of "
               "requests was the highest. By default, no such window is calculated";
    } else if (parameter == "--from" || parameter == "-f") {
        return "--from=<timestamp> | -f <timestamp> [int, >= 0, default=smallest] Ignore time before the specified";
    } else if (parameter == "--to" || parameter == "-e") {
        return "--to=<timestamp> | -e <timestamp>   [int, >= 0, default=greatest] Ignore time after the specified";
    } else if (parameter == "--output" || parameter == "-o") {
        return "--output=<path> | -o <path>         [string]                      Path to the file to which 5XX requests will be written";
    } else if (parameter == "--print" || parameter == "-p") {
        return "--print | -p                        [flag]                        If specified, 5XX requests will be printed to stdout";
    } else if (parameter == "--help" || parameter == "-h") {
        return "--help | -h                         [flag]                        Show help";
    }

    throw std::invalid_argument("Unknown parameter");
}

std::string GetHelpMessage() {
    std::stringstream message;
    
    message << "Usage: AnalyzeLog [OPTIONS] <logs_filename>\nPossible options:\n\t";
    message << GetParameterInfo("-o");
    message << "\n\t";
    message << GetParameterInfo("-p");
    message << "\n\t";
    message << GetParameterInfo("-s");
    message << "\n\t";
    message << GetParameterInfo("-w");
    message << "\n\t";
    message << GetParameterInfo("-f");
    message << "\n\t";
    message << GetParameterInfo("-e");
    message << "\n\t";
    message << GetParameterInfo("-h");

    return message.str();
}

int64_t ParseInt(const char* str) {
    char* p = nullptr;
    int64_t result = std::strtoll(str, &p, 10);

    if (*p != 0) {
        std::stringstream error_message;
        error_message << "Cannot parse an integer from \"" << str << '"';

        throw std::invalid_argument(error_message.str());
    }

    return result;
}

Parameters ParseArguments(int argc, char** argv) {
    Parameters parameters;

    bool options_terminated = false;

    for (int32_t i = 1; i < argc; ++i) {
        char* argument = argv[i];

        if (options_terminated || argument[0] != '-') {
            parameters.logs_filename = argument;
            continue;
        }

        if (std::strcmp(argument, "--") == 0) {
            options_terminated = true;
            continue;
        }

        if (std::strcmp(argument, "--output") == 0 || std::strcmp(argument, "-o") == 0) {
            if (i == argc - 1) {
                throw std::runtime_error(kMissingArgumentMsg);
            }

            parameters.output_path = argv[++i];
        } else if (std::strncmp(argument, "--output=", 9) == 0) {
            parameters.output_path = argument + 9;
        } else if (std::strcmp(argument, "--stats") == 0 || std::strcmp(argument, "-s") == 0) {
            if (i == argc - 1) {
                throw std::runtime_error(kMissingArgumentMsg);
            }

            parameters.stats = ParseInt(argv[++i]);
        } else if (std::strncmp(argument, "--stats=", 8) == 0) {
            parameters.stats = ParseInt(argument + 8);
        } else if (std::strcmp(argument, "--window") == 0 || std::strcmp(argument, "-w") == 0) {
            if (i == argc - 1) {
                throw std::runtime_error(kMissingArgumentMsg);
            }

            parameters.window = ParseInt(argv[++i]);
        } else if (std::strncmp(argument, "--window=", 9) == 0) {
            parameters.window = ParseInt(argument + 9);
        } else if (std::strcmp(argument, "--from") == 0 || std::strcmp(argument, "-f") == 0) {
            if (i == argc - 1) {
                throw std::runtime_error(kMissingArgumentMsg);
            }

            parameters.from_time = ParseInt(argv[++i]);
        } else if (std::strncmp(argument, "--from=", 7) == 0) {
            parameters.from_time = ParseInt(argument + 7);
        } else if (std::strcmp(argument, "--to") == 0 || std::strcmp(argument, "-t") == 0) {
            if (i == argc - 1) {
                throw std::runtime_error(kMissingArgumentMsg);
            }

            parameters.to_time = ParseInt(argv[++i]);
        } else if (std::strncmp(argument, "--to=", 5) == 0) {
            parameters.to_time = ParseInt(argument + 5);
        } else if (std::strcmp(argument, "--print") == 0 || std::strcmp(argument, "-p") == 0) {
            parameters.need_print = true;
        } else if (std::strcmp(argument, "--help") == 0 || std::strcmp(argument, "-h") == 0) {
            parameters.need_help = true;
        } else {
            std::stringstream error_message;
            error_message << "Unknown argument: \"" << argument << '"';

            throw std::invalid_argument(error_message.str());
        }
    }

    // TODO: agrument validation (that is not a type-check) is not the responsibility of the parser

    // std::stringstream error_message;
    // error_message << "Negative value for a positive integer argument ";

    // if (parameters.stats < 0) {
    //     error_message << "--stats (-s): " << parameters.stats;
    // } else if (parameters.window < 0) {
    //     error_message << "--window (-w): " << parameters.window;
    // } else if (parameters.from_time < 0) {
    //     error_message << "--from (-f): " << parameters.from_time;
    // } else if (parameters.to_time < 0) {
    //     error_message << "--to (-e): " << parameters.to_time;
    // } 

    // throw std::invalid_argument(error_message.str());
    
    if (parameters.logs_filename.empty() && !parameters.need_help) {
        throw std::runtime_error("No logs filename is specified");
    } else {
        return parameters;
    }
}
