#pragma once

#include "analyzing.hpp"

#include <cstdint>

struct StatsArray {
    size_t capacity = 0;
    size_t size = 0;
    RequestStatistic* data = nullptr;
};

void AddElement(StatsArray& array, RequestStatistic element);

void SortByFrequency(StatsArray& array);
