#pragma once

#include "argparsing.hpp"
#include "dynamic_arrays.hpp"

#include <cstdint>
#include <optional>

const int32_t kLineBufferSize = 16384;

struct LogEntry {
    DynamicString remote_addr;
    DynamicString request;
    DynamicString status;
    
    uint64_t timestamp = 0;
    int64_t bytes_sent = -1;
};

struct AnalyzeLogError {

};

std::optional<const char*> AnalyzeLog(const Parameters& parameters);

bool ParseLogEntry(LogEntry& to, const char* raw_entry);
