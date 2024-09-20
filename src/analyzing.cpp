#include "analyzing.hpp"

#include <cstring>

char* GetSubstring(const char* src, uint32_t from, uint32_t to) {
    if (to < from) {
        return nullptr;
    }

    char* result = new char[to - from + 1];
    result[to - from] = '\0';
    
    std::strncpy(result, src + from, to - from);

    return result;
}

int32_t FindSubstring(const char* haystack, const char* needle, int32_t start_from) {
    const char* search_result = std::strstr(haystack + start_from, needle);
    return (search_result == nullptr) ? -1 : (search_result - haystack);
}

uint32_t LocalTimeToTimestamp(const char* local_time) {
    return 42;
}

bool IsNumeric(const char* str) {
    for (int i = 0; i < std::strlen(str); ++i) {
        // '0' == 48, '9' == 57
        if (str[i] < 48 || str[i] > 57) {
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
    to.local_time = LocalTimeToTimestamp(raw_local_time);

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

    char* pos;
    int64_t bytes_sent = std::strtoll(raw_entry + bytes_sent_pos, &pos, 10);

    if (*pos != 0) {
        return false;
    }

    to.bytes_sent = bytes_sent;

    delete[] raw_local_time;
    return true;
}
