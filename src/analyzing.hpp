#pragma once

#include "argparsing.hpp"

#include <cstdint>

#include <string>

struct LogEntry {
    char* remote_addr = nullptr;
    char* request = nullptr;
    char* status = nullptr;
    
    uint32_t timestamp = 0;
    int64_t bytes_sent = -1;
};

struct RequestStatistic {
    char* request = nullptr;
    uint64_t frequency = 0;
};

void Analyze(const Parameters& parameters);

//bool ParseLogEntry(LogEntry& to, std::string_view raw_entry);

bool ParseLogEntry(LogEntry& to, const char* raw_entry);
