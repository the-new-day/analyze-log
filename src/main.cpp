#include <iostream>

#include "arguments.hpp"

int main(int argc, char** argv){
    Parameters params = ParseArgs(argc, argv);

    std::cout << "Output file: " << params.output_path << '\n'
              << "Print: " << (params.need_print ? "true" : "false") << '\n'
              << "Stats: " << params.stats << '\n'
              << "Window: " << params.window << '\n'
              << "From: " << params.from_time << '\n'
              << "To: " << params.to_time << '\n'
              << "Logs file: " << params.logs_filename << '\n';

    return EXIT_SUCCESS;
}
