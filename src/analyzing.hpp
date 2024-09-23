#pragma once

#include "argparsing.hpp"
#include "datetime.hpp"

#include <cstdint>

struct LogEntry {
    std::string_view remote_addr;
    std::string_view request;
    std::string_view status;
    
    uint32_t timestamp = 0;
    int64_t bytes_sent = -1;
};

void Analyze(const Parameters& parameters);

bool ParseLogEntry(LogEntry& to, std::string_view raw_entry);
