#include <iostream>

#include "argparsing.hpp"

int main(int argc, char** argv){
    try {
        Parameters params = ParseArguments(argc, argv);
        if (params.need_help) {
            std::cout << GetHelpMessage();
            return EXIT_SUCCESS;
        }

        std::cout << "Output file: " << params.output_path << '\n'
                << "Print: " << (params.need_print ? "true" : "false") << '\n'
                << "Stats: " << params.stats << '\n'
                << "Window: " << params.window << '\n'
                << "From: " << params.from_time << '\n'
                << "To: " << params.to_time << '\n'
                << "Logs file: " << params.logs_filename << '\n';
    } catch(const std::exception& e) {
        std::cerr << "An error occured:\n" << e.what();
        std::cout << "\nUse --help to see information about supported commands";

        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}
