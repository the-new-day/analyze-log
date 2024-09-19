#pragma once

#include "argparsing.hpp"

#include <string>
#include <cstdint>

struct LogEntry {
    std::string remote_addr;
    uint32_t local_time;
    std::string request;
    uint16_t status;
    uint32_t bytes;
};

void Analyze(const Parameters& parameters);

LogEntry ParseLogEntry(const std::string& entry);
