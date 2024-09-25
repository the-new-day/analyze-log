#include "analyzing.hpp"
#include "stats_array.hpp"
#include "datetime.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <sstream>

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
        std::stringstream error_message;
        error_message << "Cannot find file \"" << parameters.logs_filename << '"';

        throw std::invalid_argument(error_message.str());
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
    std::cout << "\n[5XX requests statistics]:\n";

    for (int i = 0; i < amount && i < stats.size; ++i) {
        std::cout << "* " << stats.data[i].request << " - " 
                  << stats.data[i].frequency << " request" << (stats.data[i].frequency > 1 ? "s" : "") << '\n';
    }

    if (stats.size == 0) {
        std::cout << "No such requests found\n";
    }
}

void ShiftArray(uint32_t* array, uint32_t n) {
    for (uint32_t i = 0; i < n - 1; ++i) {
        array[i] = array[i + 1];
    }
}

void AnalyzeLog(const Parameters& parameters) {
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

    uint64_t lower_timestamp = 0;
    uint64_t higher_timestamp;

    uint32_t max_amount_of_requests = 0;
    uint32_t current_amount_of_requests = 0;
    uint32_t amount_of_requests_on_lower = 0;

    uint64_t result_lower_timestamp = 0;
    uint64_t result_higher_timestamp = 0;

    char* line_buffer = new char[kLineBufferSize];

    uint32_t* amount_of_requests_in_second = new uint32_t[parameters.window];
    std::fill(amount_of_requests_in_second, amount_of_requests_in_second + parameters.window, 0);
    
    while (input_file.getline(line_buffer, kLineBufferSize)) {
        LogEntry entry;
        
        if (!ParseLogEntry(entry, line_buffer)) {
            if (parameters.invalid_lines_output_path != nullptr) {
                invalid_lines_output_file << line_buffer << '\n';
            }

            if (entry.remote_addr != nullptr) {
                delete[] entry.remote_addr;
            }

            if (entry.request != nullptr) {
                delete[] entry.request;
            }

            if (entry.status != nullptr) {
                delete[] entry.status;
            }

            continue;
        }

        if (parameters.from_time > entry.timestamp) {
            continue;
        } else if (parameters.to_time != 0 && parameters.to_time < entry.timestamp) {
            break;
        }

        if (parameters.window != 0) {
            if (lower_timestamp == 0) {
                lower_timestamp = entry.timestamp;
                higher_timestamp = lower_timestamp + parameters.window;
            }

            if (entry.timestamp <= higher_timestamp) {
                ++amount_of_requests_in_second[entry.timestamp - lower_timestamp];
                ++current_amount_of_requests;
            } else {
                if (max_amount_of_requests < current_amount_of_requests) {
                    max_amount_of_requests = current_amount_of_requests;
                    result_higher_timestamp = higher_timestamp;
                    result_lower_timestamp = lower_timestamp;
                }

                ShiftArray(amount_of_requests_in_second, parameters.window);
                ++lower_timestamp;
                ++higher_timestamp;
            }
        }

        if (parameters.stats > 0 && entry.status[0] == '5') {
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

    if (current_amount_of_requests > max_amount_of_requests) {
        max_amount_of_requests = current_amount_of_requests;
        result_lower_timestamp = lower_timestamp;
        result_higher_timestamp = higher_timestamp;
    }

    delete[] line_buffer;
    delete[] amount_of_requests_in_second;

    if (parameters.stats > 0) {
        SortByFrequency(error_logs_stats);
        PrintStats(error_logs_stats, parameters.stats);
    }

    if (error_logs_stats.data != nullptr) {
        delete[] error_logs_stats.data;
    }

    if (parameters.window > 0) {
        std::cout << "\n[Window]:\n";
        std::cout << result_lower_timestamp << " - " << result_higher_timestamp << " (";
        std::cout << max_amount_of_requests << " request" << (max_amount_of_requests > 1 ? "s)" : ")");
    }
}

char* GetSubstring(const char* src, uint32_t from, uint32_t to) {
    if (to < from) {
        return nullptr;
    }
    
    char* result = new char[to - from + 1];
    std::strncpy(result, src + from, to - from);
    result[to - from] = '\0';

    return result;
}

int32_t FindSubstring(const char* haystack, const char* needle, int32_t start_from = 0) {
    const char* search_result = std::strstr(haystack + start_from, needle);
    return (search_result == nullptr) ? -1 : (search_result - haystack);
}

bool IsNumeric(const char* str) {
    for (int i = 0; i < std::strlen(str); ++i) {
        if (!std::isdigit(str[i])) {
            return false;
        }
    }

    return true;
}

bool ParseLogEntry(LogEntry& to, const char* raw_entry) {
    int32_t remote_addr_length = FindSubstring(raw_entry, " - - ");

    if (remote_addr_length == -1) {
        return false;
    }

    to.remote_addr = GetSubstring(raw_entry, 0, remote_addr_length);

    int32_t local_time_start = remote_addr_length + std::strlen(" - - ");
    int32_t local_time_end = FindSubstring(raw_entry, "]", local_time_start);

    if (raw_entry[local_time_start] != '[' || local_time_end == -1) {
        return false;
    }

    char* raw_local_time = GetSubstring(raw_entry, local_time_start + 1, local_time_end);
    to.timestamp = LocalTimeStringToTimestamp(raw_local_time);
    delete[] raw_local_time;

    if (to.timestamp == 0) {
        return false;
    }

    int32_t request_start = local_time_end + 2;
    int32_t request_end = FindSubstring(raw_entry, "\"", request_start + 1);
    
    if (raw_entry[request_start] != '"' || raw_entry[request_start - 1] != ' ' || request_end == -1) {
        return false;
    }

    to.request = GetSubstring(raw_entry, request_start + 1, request_end);

    int32_t status_start = request_end + 2;
    int32_t status_end = FindSubstring(raw_entry, " ", status_start);
    
    if (raw_entry[status_start - 1] != ' ' || status_end == -1 || status_end - status_start != 3) {
        return false;
    }

    char* raw_status = GetSubstring(raw_entry, status_start, status_end);

    if (!IsNumeric(raw_status)) {
        delete[] raw_status;
        return false;
    }

    to.status = raw_status;

    int32_t bytes_sent_start = status_end + 1;

    if (raw_entry[bytes_sent_start - 1] != ' ') {
        return false;
    }

    int64_t bytes_sent;

    if (std::strlen(raw_entry + bytes_sent_start) == 1 && raw_entry[bytes_sent_start] == '-') {
        bytes_sent = 0;
    } else {
        char* pos;
        bytes_sent = std::strtoll(raw_entry + bytes_sent_start, &pos, 10);

        if (*pos != 0) {
            delete[] raw_local_time;
            return false;
        }
    }

    to.bytes_sent = bytes_sent;

    return true;
}
