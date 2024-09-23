#include <iostream>

#include "analyzing.hpp"

int monthToNumber(const char *month) {
    if (strcmp(month, "Jan") == 0) return 1;
    if (strcmp(month, "Feb") == 0) return 2;
    if (strcmp(month, "Mar") == 0) return 3;
    if (strcmp(month, "Apr") == 0) return 4;
    if (strcmp(month, "May") == 0) return 5;
    if (strcmp(month, "Jun") == 0) return 6;
    if (strcmp(month, "Jul") == 0) return 7;
    if (strcmp(month, "Aug") == 0) return 8;
    if (strcmp(month, "Sep") == 0) return 9;
    if (strcmp(month, "Oct") == 0) return 10;
    if (strcmp(month, "Nov") == 0) return 11;
    if (strcmp(month, "Dec") == 0) return 12;
    return -1; // Некорректный месяц
}

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

        // std::cout << "Output file: " << params.output_path << '\n'
        //         << "Print: " << (params.need_print ? "true" : "false") << '\n'
        //         << "Stats: " << params.stats << '\n'
        //         << "Window: " << params.window << '\n'
        //         << "From: " << params.from_time << '\n'
        //         << "To: " << params.to_time << '\n'
        //         << "Logs file: " << params.logs_filename << '\n';

        // const char* raw_entry = "199.72.81.55 - - [01/Jul/1995:00:00:01 -0400] \"GET /history/apollo/ HTTP/1.0\" 200 6245";
        // LogEntry entry; 
        // ParseLogEntry(entry, raw_entry);

        // std::cout << "remote_addr: " << entry.remote_addr << '\n'
        //         << "local_time: " << entry.timestamp << '\n'
        //         << "request: " << entry.request << '\n'
        //         << "status: " << entry.status << '\n'
        //         << "bytes: " << entry.bytes_sent << '\n';

        Analyze(params);
    } catch (const std::exception& e) {
        std::cerr << "An error occured:\n" << e.what();
        std::cout << "\nUse --help to see information about supported commands";

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
