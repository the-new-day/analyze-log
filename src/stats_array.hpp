#pragma once

#include "analyzing.hpp"

#include <cstdint>

struct StatsArray {
    uint64_t capacity = 0;
    uint64_t size = 0;
    RequestStatistic* data = nullptr;
};

void AddElement(StatsArray& array, RequestStatistic element);

void SortByFrequency(StatsArray& array);
