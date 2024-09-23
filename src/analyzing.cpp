#include "analyzing.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

void OutputLine(std::string_view line, std::ostream& stream) {
    stream << line;
}

void ValidateParameters(const Parameters& parameters) {
    std::stringstream negative_value_error_message;
    negative_value_error_message << "Negative value for a positive integer argument ";

    if (parameters.stats < 0) {
        negative_value_error_message << "--stats (-s): " << parameters.stats;
        throw std::invalid_argument(negative_value_error_message.str());
    } else if (parameters.window < 0) {
        negative_value_error_message << "--window (-w): " << parameters.window;
        throw std::invalid_argument(negative_value_error_message.str());
    } else if (parameters.from_time < 0) {
        negative_value_error_message << "--from (-f): " << parameters.from_time;
        throw std::invalid_argument(negative_value_error_message.str());
    } else if (parameters.to_time < 0) {
        negative_value_error_message << "--to (-e): " << parameters.to_time;
        throw std::invalid_argument(negative_value_error_message.str());
    }

    if (!parameters.logs_filename.empty() && !std::filesystem::exists(parameters.logs_filename)) {
        throw std::invalid_argument("Unknown input file");
    }
}

void Analyze(const Parameters& parameters) {
    ValidateParameters(parameters);

    std::ifstream input_file(std::string(parameters.logs_filename));
    if (input_file.fail()) {
        throw std::runtime_error("Unable to read the input file");
    }

    std::ofstream output_file;

    if (!parameters.output_path.empty()) {
        output_file = std::ofstream(std::string(parameters.output_path));
    }
    
    std::string line;
    while (std::getline(input_file, line)) {
        LogEntry entry;
        if (!ParseLogEntry(entry, line)) {
            continue;
        }

        if (parameters.from_time > entry.timestamp) {
            continue;
        } else if (parameters.to_time != 0 && parameters.to_time < entry.timestamp) {
            break;
        }

        if (!parameters.output_path.empty() && entry.status.starts_with('5')) {
            OutputLine(line + '\n', output_file);
            if (parameters.need_print) {
                OutputLine(line + '\n', std::cout);
            }
        }
    }
}

bool IsNumeric(std::string_view str) {
    for (int i = 0; i < str.length(); ++i) {
        if (str[i] < '0' || str[i] > '9') {
            return false;
        }
    }

    return true;
}

bool ParseLogEntry(LogEntry& to, std::string_view raw_entry) {
    if (raw_entry.find(" - - ") == std::string_view::npos) {
        return false;
    }

    to.remote_addr = raw_entry.substr(0, raw_entry.find(" - - "));

    raw_entry = raw_entry.substr(raw_entry.find(" - - ") + 5);
    int32_t time_end_pos = raw_entry.find(']');

    if (raw_entry[0] != '[' || time_end_pos == std::string_view::npos) {
        return false;
    }

    to.timestamp = LocalTimeStringToTimestamp(raw_entry.substr(1, time_end_pos - 1));

    if (raw_entry[time_end_pos + 1] != ' ' || raw_entry[time_end_pos + 2] != '"') {
        return false;
    }

    raw_entry = raw_entry.substr(time_end_pos + 2);

    to.request = raw_entry.substr(raw_entry.find_first_of('"') + 1, raw_entry.find_last_of('"') - raw_entry.find_first_of('"') - 1);

    raw_entry = raw_entry.substr(to.request.length() + 3);

    to.status = raw_entry.substr(0, raw_entry.find(' '));

    if (to.status.length() != 3 || !IsNumeric(to.status)) {
        return false;
    }
    
    raw_entry = raw_entry.substr(to.status.length() + 1);

    int64_t bytes_sent;
    std::from_chars_result convertion_result = std::from_chars(raw_entry.data(), raw_entry.data() + raw_entry.size(), bytes_sent);

    if (convertion_result.ec == std::errc::invalid_argument || convertion_result.ptr != raw_entry.end()) {
        return false;
    }

    to.bytes_sent = bytes_sent;

    return true;
}
