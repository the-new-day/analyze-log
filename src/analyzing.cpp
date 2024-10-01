#include "analyzing.hpp"
#include "dynamic_arrays.hpp"
#include "datetime.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <sstream>

void UpdateStatistics(StatsArray& statistics, const char* request) {
    for (size_t i = 0; i < statistics.size; ++i) {
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

    for (size_t i = 0; i < amount && i < stats.size; ++i) {
        std::cout << "* " << stats.data[i].request << " - " 
                  << stats.data[i].frequency << " request" << (stats.data[i].frequency == 1 ? "" : "s") << '\n';
    }

    if (stats.size == 0) {
        std::cout << "No such requests found\n";
    }
}

void PrintWindow(uint64_t lower_timestamp, uint64_t higher_timestamp, uint32_t amount_of_requests) {
    char higher_time[27];
    char lower_time[27];

    TimestampToDateTimeString(higher_timestamp, higher_time);
    TimestampToDateTimeString(lower_timestamp, lower_time);

    std::cout << "\n[Window]:\n";
    std::cout << '[' << lower_time << "] - [" << higher_time << "] (";
    std::cout << amount_of_requests << " request" << (amount_of_requests == 1 ? ")" : "s)");
}

std::optional<const char*> AnalyzeLog(const Parameters& parameters) {
    std::ifstream input_file(parameters.logs_filename);
    if (input_file.fail()) {
        return "Unable to read the input file";
    }

    std::ofstream output_file;
    if (parameters.output_path != nullptr) {
        output_file = std::ofstream(parameters.output_path);
        if (output_file.fail()) {
            return "Unable to open the output file";
        }
    }

    std::ofstream invalid_lines_output_file;
    if (parameters.invalid_lines_output_path != nullptr) {
        invalid_lines_output_file = std::ofstream(parameters.invalid_lines_output_path);
        if (output_file.fail()) {
            return "Unable to open the invalid lines output file";
        }
    }

    StatsArray error_logs_stats;

    // calculation of the "window"
    uint64_t lower_timestamp = 0;
    uint64_t higher_timestamp;

    uint32_t max_amount_of_requests = 0;
    uint32_t current_amount_of_requests = 0;

    uint64_t result_lower_timestamp = 0;
    uint64_t result_higher_timestamp = 0;

    uint64_t last_timestamp;

    uint32_t* amount_of_requests_in_second = new uint32_t[parameters.window];
    std::fill(amount_of_requests_in_second, amount_of_requests_in_second + parameters.window, 0);

    uint32_t array_offset = 0;

    char* line_buffer = new char[kLineBufferSize];

    LogEntry entry;

    uint64_t lines_analyzed = 0;
    uint64_t invalid_lines_amount = 0;
    uint64_t server_error_lines_amount = 0;
    uint64_t too_long_lines_amount = 0;

    while (input_file.good()) {
        input_file.getline(line_buffer, kLineBufferSize);
        ++lines_analyzed;

        if (input_file.fail()) {
            if (input_file.eof()) {
                --lines_analyzed;
                break;
            }

            input_file.clear();
            input_file.ignore(UINT64_MAX, '\n');
            ++too_long_lines_amount;
            continue;
        }

        if (!ParseLogEntry(entry, line_buffer)) {
            if (parameters.invalid_lines_output_path != nullptr) {
                invalid_lines_output_file << line_buffer << std::endl;
            }

            ++invalid_lines_amount;
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
                higher_timestamp = lower_timestamp + parameters.window - 1;
            }

            if (entry.timestamp >= lower_timestamp && entry.timestamp <= higher_timestamp) {
                ++amount_of_requests_in_second[(array_offset + entry.timestamp - lower_timestamp) % parameters.window];
            } else {
                if (current_amount_of_requests > max_amount_of_requests) {
                    max_amount_of_requests = current_amount_of_requests;
                    result_higher_timestamp = higher_timestamp;
                    result_lower_timestamp = lower_timestamp;
                }
                
                higher_timestamp = entry.timestamp;

                while (lower_timestamp + parameters.window <= higher_timestamp) {
                    current_amount_of_requests -= amount_of_requests_in_second[array_offset % parameters.window];
                    amount_of_requests_in_second[array_offset % parameters.window] = 0;

                    ++array_offset;
                    ++lower_timestamp;
                }

                ++amount_of_requests_in_second[(array_offset + parameters.window - 1) % parameters.window];
            }

            ++current_amount_of_requests;
        }

        if (entry.status.data[0] == '5') {
            ++server_error_lines_amount;
        }

        if (parameters.output_path != nullptr && parameters.stats > 0 && entry.status.data[0] == '5') {
            UpdateStatistics(error_logs_stats, entry.request.data);
        }

        if (parameters.output_path != nullptr && entry.status.data[0] == '5') {
            output_file << line_buffer << std::endl;
            if (parameters.need_print) {
                std::cout << line_buffer << std::endl;
            }
        }

        last_timestamp = entry.timestamp;
    }

    std::cout << "Analyzed " << lines_analyzed << " lines, " << server_error_lines_amount << " were with the code 5XX, "
        << invalid_lines_amount << " were invalid, " << too_long_lines_amount << " were ignored (too long)." << std::endl;

    if (entry.remote_addr.data != nullptr) {
        delete[] entry.remote_addr.data;
    }

    if (entry.request.data != nullptr) {
        delete[] entry.request.data;
    }

    if (entry.status.data != nullptr) {
        delete[] entry.status.data;
    }

    delete[] line_buffer;
    delete[] amount_of_requests_in_second;

    if (current_amount_of_requests > max_amount_of_requests) {
        max_amount_of_requests = current_amount_of_requests;
        result_higher_timestamp = higher_timestamp;
        result_lower_timestamp = lower_timestamp;
    }

    if (result_higher_timestamp > last_timestamp) {
        result_higher_timestamp = last_timestamp;
    }

    if (parameters.output_path != nullptr && parameters.stats > 0) {
        SortByFrequency(error_logs_stats);
        PrintStats(error_logs_stats, parameters.stats);
    }

    if (error_logs_stats.data != nullptr) {
        delete[] error_logs_stats.data;
    }

    if (parameters.window > 0) {
        PrintWindow(result_lower_timestamp, result_higher_timestamp, max_amount_of_requests);
    }

    return std::nullopt;
}

std::optional<size_t> FindSubstring(const char* haystack, const char* needle, size_t start_from = 0) {
    const char* search_result = std::strstr(haystack + start_from, needle);
    if (search_result == nullptr) {
        return std::nullopt;
    }

    return search_result - haystack;
}

bool IsNumeric(const char* str) {
    for (size_t i = 0; i < std::strlen(str); ++i) {
        if (!std::isdigit(str[i])) {
            return false;
        }
    }

    return true;
}

bool ParseLogEntry(LogEntry& to, const char* raw_entry) {
    std::optional<size_t> remote_addr_length = FindSubstring(raw_entry, " - - ");

    if (!remote_addr_length.has_value()) {
        return false;
    }

    SetString(to.remote_addr, raw_entry, remote_addr_length.value());

    size_t local_time_start = remote_addr_length.value() + std::strlen(" - - ");
    std::optional<size_t> local_time_end = FindSubstring(raw_entry, "]", local_time_start);

    if (raw_entry[local_time_start] != '[' || !local_time_end.has_value()) {
        return false;
    }

    size_t local_time_length = local_time_end.value() - local_time_start - 1;
    std::string_view raw_local_time = std::string_view(raw_entry + local_time_start + 1, local_time_length);

    std::optional<uint64_t> timestamp = LocalTimeStringToTimestamp(raw_local_time);
    if (!timestamp.has_value()) {
        return false;
    }

    to.timestamp = timestamp.value();

    if (to.timestamp == 0) {
        return false;
    }

    size_t request_start = local_time_end.value() + 2;
    std::optional<size_t> request_end = FindSubstring(raw_entry, "\"", request_start + 1);
    
    if (raw_entry[request_start] != '"' || raw_entry[request_start - 1] != ' ' || !request_end.has_value()) {
        return false;
    }

    SetString(to.request, raw_entry + request_start + 1, (request_end.value() - request_start - 1));

    size_t status_start = request_end.value() + 2;
    std::optional<size_t> status_end = FindSubstring(raw_entry, " ", status_start);
    
    if (raw_entry[status_start - 1] != ' ' || !status_end.has_value() || status_end.value() - status_start != 3) {
        return false;
    }

    SetString(to.status, raw_entry + status_start, (status_end.value() - status_start));

    if (!IsNumeric(to.status.data)) {
        return false;
    }

    size_t bytes_sent_start = status_end.value() + 1;

    if (raw_entry[bytes_sent_start - 1] != ' ') {
        return false;
    }

    if (std::strlen(raw_entry + bytes_sent_start) == 1 && raw_entry[bytes_sent_start] == '-') {
        to.bytes_sent = 0;
    } else {
        std::expected<int64_t, const char*> bytes_sent = ParseInt(raw_entry + bytes_sent_start);
        if (!bytes_sent.has_value()) {
            return false;
        }

        to.bytes_sent = bytes_sent.value();
    }

    return true;
}
