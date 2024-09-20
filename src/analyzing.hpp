#pragma once

#include "argparsing.hpp"

#include <cstdint>

struct LogEntry {
    char* remote_addr{nullptr};
    uint32_t local_time{0};
    char* request{nullptr};
    char* status{nullptr};
    int64_t bytes_sent{-1};
};

void Analyze(const Parameters& parameters);

bool ParseLogEntry(LogEntry& to, const char* raw_entry);

int32_t FindSubstring(const char* haystack, const char* needle, int32_t start_from = 0);
