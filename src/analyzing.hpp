#pragma once

#include "argparsing.hpp"

#include <cstdint>

const int32_t kLineBufferSize = 8192;

struct LogEntry {
    char* remote_addr = nullptr;
    char* request = nullptr;
    char* status = nullptr;
    
    uint64_t timestamp = 0;
    int64_t bytes_sent = -1;
};

struct RequestStatistic {
    char* request = nullptr;
    uint64_t frequency = 0;
};

void AnalyzeLog(const Parameters& parameters);

bool ParseLogEntry(LogEntry& to, const char* raw_entry);
