#include "arguments.hpp"

#include <cstring>
#include <cstdlib>

uint32_t ParseNumericArg(const char* str) {
    char* p;
    uint32_t result = std::strtoll(str, &p, 10);

    if (p != nullptr || result < 0) {
        // TODO: error
    }

    return result;
}

Parameters ParseArgs(int argc, char** argv) {
    Parameters parameters;

    for (int32_t i = 1; i < argc; ++i) {
        char* argument = argv[i];

        if (std::strcmp(argument, "-o") == 0) {
            if (i == argc - 1) {
                // TODO: error
            }

            parameters.output_path = argv[++i];
        } else if (std::strncmp(argument, "--output=", 9) == 0) {
            parameters.output_path = argument + 9;
        } else if (std::strcmp(argument, "-p") == 0 || std::strcmp(argument, "--print") == 0) {
            parameters.need_print = true;
        } else if (std::strcmp(argument, "-s") == 0) {
            if (i == argc - 1) {
                // TODO: error
            }

            parameters.stats = ParseNumericArg(argv[++i]);
        } else if (strncmp(argument, "--stats=", 8) == 0) {
            parameters.stats = ParseNumericArg(argument + 8);
        }  else if (std::strcmp(argument, "-w") == 0) {
            if (i == argc - 1) {
                // TODO: error
            }

            parameters.window = ParseNumericArg(argv[++i]);
        } else if (strncmp(argument, "--window=", 9) == 0) {
            parameters.window = ParseNumericArg(argument + 9);
        }  else if (std::strcmp(argument, "-f") == 0) {
            if (i == argc - 1) {
                // TODO: error
            }

            parameters.from_time = ParseNumericArg(argv[++i]);
        } else if (strncmp(argument, "--from=", 7) == 0) {
            parameters.from_time = ParseNumericArg(argument + 7);
        }  else if (std::strcmp(argument, "-e") == 0) {
            if (i == argc - 1) {
                // TODO: error
            }

            parameters.to_time = ParseNumericArg(argv[++i]);
        } else if (strncmp(argument, "--to=", 9) == 0) {
            parameters.to_time = ParseNumericArg(argument + 5);
        } else {
            parameters.logs_filename = argument;
        }
    }

    return parameters;
}
