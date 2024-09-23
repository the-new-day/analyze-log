#include <iostream>

#include "analyzing.hpp"

int main(int argc, char** argv){
    try {
        if (argc < 2) {
            ShowHelpMessage();
            return EXIT_SUCCESS;
        }

        Parameters params = ParseArguments(argc, argv);
        if (params.need_help) {
            ShowHelpMessage();
            return EXIT_SUCCESS;
        }

        Analyze(params);
    } catch (const std::exception& e) {
        std::cerr << "An error occured:\n" << e.what();
        std::cout << "\nUse --help to see information about supported commands";

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
