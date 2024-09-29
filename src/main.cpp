#include <iostream>
#include <optional>
#include <expected>

#include "analyzing.hpp"

int main(int argc, char** argv){
    if (argc < 2) {
        ShowHelpMessage();
        return EXIT_SUCCESS;
    }

    std::expected<Parameters, ParametersParseError> params = ParseArguments(argc, argv);
    if (!params.has_value()) {
        ParametersParseError error = params.error();

        std::cerr << "An error occured while parsing arguments:\n" << error.message;
        if (error.argument != nullptr) {
            std::cerr << '\n' << error.argument;
        }

        std::cout << "\nUse --help to see information about supported commands";

        return EXIT_FAILURE;
    }

    if (params->need_help) {
        ShowHelpMessage();
        return EXIT_SUCCESS;
    }

    std::optional<const char*> analyzing_error = AnalyzeLog(params.value());
    
    if (analyzing_error.has_value()) {
        std::cerr << "An error occured while analyzing the file:\n" << analyzing_error.value();

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
