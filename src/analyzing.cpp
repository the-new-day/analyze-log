#include "analyzing.hpp"

#include <cstring>

#include <iostream>

void Analyze(const Parameters& parameters) {

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
    
    raw_entry = raw_entry.substr(to.status.length() + 1);

    int64_t bytes_sent;
    std::from_chars_result convertion_result = std::from_chars(raw_entry.data(), raw_entry.data() + raw_entry.size(), bytes_sent);

    if (convertion_result.ec == std::errc::invalid_argument || convertion_result.ptr != raw_entry.end()) {
        return false;
    }

    to.bytes_sent = bytes_sent;

    return true;
}
