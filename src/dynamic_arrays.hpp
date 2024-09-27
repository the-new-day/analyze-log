#pragma once

#include <cstdint>
#include <cstddef>

struct RequestStatistic {
    char* request = nullptr;
    uint64_t frequency = 0;
};

struct StatsArray {
    size_t capacity = 0;
    size_t size = 0;
    RequestStatistic* data = nullptr;
};

struct DynamicString {
    char* data = nullptr;
    size_t capacity = 0;
    size_t size = 0;
};

void AddElement(StatsArray& array, RequestStatistic element);

void SortByFrequency(StatsArray& array);

void SetString(DynamicString& string, const char* src, size_t length);
