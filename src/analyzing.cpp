#include "analyzing.hpp"
#include "stats_array.hpp"
#include "datetime.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>

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
        negative_value_error_message << "--to (-t): " << parameters.to_time;
        throw std::invalid_argument(negative_value_error_message.str());
    }

    if (parameters.logs_filename != nullptr && !std::filesystem::exists(parameters.logs_filename)) {
        throw std::invalid_argument("Unknown input file");
    }
}

void UpdateStatistics(StatsArray& statistics, const char* request) {
    for (uint32_t i = 0; i < statistics.size; ++i) {
        if (std::strcmp(statistics.data[i].request, request) == 0) {
            ++statistics.data[i].frequency;
            return;
        }
    }

    RequestStatistic stat;
    stat.frequency = 1;
    stat.request = new char[std::strlen(request) + 1];
    std::strcpy(stat.request, request);

    AddElement(statistics, stat);
}

void PrintStats(const StatsArray& stats, int32_t amount) {
    std::cout << "[5XX requests statistics]:\n\n";

    for (int i = 0; i < amount && i < stats.size; ++i) {
        std::cout << stats.data[i].frequency << ' ' << stats.data[i].request << '\n';
    }
}

void Analyze(const Parameters& parameters) {
    ValidateParameters(parameters);

    std::ifstream input_file(parameters.logs_filename);
    if (input_file.fail()) {
        throw std::runtime_error("Unable to read the input file");
    }

    std::ofstream output_file;
    if (parameters.output_path != nullptr) {
        output_file = std::ofstream(parameters.output_path);
        if (output_file.fail()) {
            throw std::runtime_error("Unable to open the output file");
        }
    }

    std::ofstream invalid_lines_output_file;
    if (parameters.invalid_lines_output_path != nullptr) {
        invalid_lines_output_file = std::ofstream(parameters.invalid_lines_output_path);
        if (output_file.fail()) {
            throw std::runtime_error("Unable to open the invalid lines output file");
        }
    }

    StatsArray error_logs_stats;

    char line_buffer[8192];
    
    while (input_file.getline(line_buffer, 8192)) {
        LogEntry entry;
        
        if (!ParseLogEntry(entry, line_buffer)) {
            if (parameters.invalid_lines_output_path != nullptr) {
                invalid_lines_output_file << line_buffer << '\n';
            }

            continue;
        }

        if (parameters.from_time > entry.timestamp) {
            continue;
        } else if (parameters.to_time != 0 && parameters.to_time < entry.timestamp) {
            break;
        }

        if (entry.status[0] == '5') {
            UpdateStatistics(error_logs_stats, entry.request);
        }

        if (parameters.output_path != nullptr && entry.status[0] == '5') {
            output_file << line_buffer << '\n';
            if (parameters.need_print) {
                std::cout << line_buffer << '\n';
            }
        }

        delete[] entry.remote_addr;
        delete[] entry.request;
        delete[] entry.status;
    }

    SortByFrequency(error_logs_stats);
    PrintStats(error_logs_stats, parameters.stats);
}

char* GetSubstring(const char* src, uint32_t from, uint32_t to) {
    if (to < from) {
        return nullptr;
    }
    
    char* result = new char[to - from + 1];
    result[to - from] = '\0';
    
    std::strncpy(result, src + from, to - from);
    return result;
}

int32_t FindSubstring(const char* haystack, const char* needle, int32_t start_from = 0) {
    const char* search_result = std::strstr(haystack + start_from, needle);
    return (search_result == nullptr) ? -1 : (search_result - haystack);
}

bool IsNumeric(const char* str) {
    for (int i = 0; i < std::strlen(str); ++i) {
        if (str[i] < '0' || str[i] > '9') {
            return false;
        }
    }

    return true;
}

bool ParseLogEntry(LogEntry& to, const char* raw_entry) {
    if (FindSubstring(raw_entry, " - - ") == -1) {
        return false;
    }

    to.remote_addr = GetSubstring(raw_entry, 0, FindSubstring(raw_entry, " - - "));

    int32_t local_time_pos = std::strlen(to.remote_addr) + std::strlen(" - - ");

    if (raw_entry[local_time_pos] != '[' || FindSubstring(raw_entry, "]", local_time_pos) == -1) {
        return false;
    }

    char* raw_local_time = GetSubstring(raw_entry, local_time_pos + 1, FindSubstring(raw_entry, "]", local_time_pos));
    to.timestamp = LocalTimeStringToTimestamp(raw_local_time);

    int32_t request_pos = local_time_pos + std::strlen(raw_local_time) + 3;
    if (raw_entry[request_pos] != '"' 
        || raw_entry[request_pos - 1] != ' ' 
        || FindSubstring(raw_entry, "\"", request_pos + 1) == -1)
    {
        return false;
    }

    to.request = GetSubstring(raw_entry, request_pos + 1, FindSubstring(raw_entry, "\"", request_pos + 1));
    int32_t status_pos = request_pos + std::strlen(to.request) + 3;
    
    if (raw_entry[status_pos - 1] != ' ' 
        || FindSubstring(raw_entry, " ", status_pos) == -1
        || FindSubstring(raw_entry, " ", status_pos) - status_pos != 3) 
    {
        return false;
    }

    char* raw_status = GetSubstring(raw_entry, status_pos, FindSubstring(raw_entry, " ", status_pos));

    if (!IsNumeric(raw_status)) {
        delete[] raw_status;
        return false;
    }

    to.status = raw_status;

    int32_t bytes_sent_pos = status_pos + std::strlen(to.status) + 1;

    if (raw_entry[bytes_sent_pos - 1] != ' ') {
        return false;
    }

    int64_t bytes_sent;

    if (std::strlen(raw_entry + bytes_sent_pos) == 1 && raw_entry[bytes_sent_pos] == '-') {
        bytes_sent = 0;
    } else {
        char* pos;
        bytes_sent = std::strtoll(raw_entry + bytes_sent_pos, &pos, 10);

        if (*pos != 0) {
            delete[] raw_local_time;
            return false;
        }
    }

    to.bytes_sent = bytes_sent;

    delete[] raw_local_time;
    return true;
}
