#include <iostream>

#include "argparsing.hpp"
#include "analyzer.hpp"

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

        // std::string raw_entry = "199.72.81.55 - - [01/Jul/1995:00:00:01 -0400] \"GET /history/apollo/ HTTP/1.0\" 200 6245";
        // LogEntry entry = ParseLogEntry(raw_entry);

        // std::cout << "remote_addr: " << entry.remote_addr << '\n'
        //         << "local_time: " << entry.local_time << '\n'
        //         << "request: " << entry.request << '\n'
        //         << "status: " << entry.status << '\n'
        //         << "bytes: " << entry.bytes << '\n';
    } catch (const std::exception& e) {
        std::cerr << "An error occured:\n" << e.what();
        std::cout << "\nUse --help to see information about supported commands";

        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}
