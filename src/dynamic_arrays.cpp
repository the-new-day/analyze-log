#include "dynamic_arrays.hpp"

#include <iostream>
#include <cstring>

void AddElement(StatsArray& array, RequestStatistic element) {
    if (array.size == array.capacity) {
        if (array.capacity == 0) {
            array.capacity = 1;
        }

        RequestStatistic* new_data = new RequestStatistic[array.capacity * 2];
        array.capacity *= 2;

        for (size_t i = 0; i < array.size; ++i) {
            new_data[i] = array.data[i];
        }

        if (array.data != nullptr) {
            delete[] array.data;
        }

        array.data = new_data;
    }

    array.data[array.size] = element;
    ++array.size;
}

void SortByFrequency(RequestStatistic* data, int32_t low, int32_t high) {
    int32_t i = low;
    int32_t j = high;
    uint64_t pivot = data[(i + j) / 2].frequency;

    while (i <= j) {
        while (data[i].frequency > pivot) {
            ++i;
        }

        while (data[j].frequency < pivot) {
            --j;
        }

        if (i > j) {
            continue;
        }

        std::swap(data[i], data[j]);
        ++i;
        --j;
    }

    if (j > low) {
        SortByFrequency(data, low, j);
    }

    if (i < high) {
        SortByFrequency(data, i, high);
    }
}

void SortByFrequency(StatsArray& array) {
    if (array.size < 2) {
        return;
    }
    
    SortByFrequency(array.data, 0, array.size - 1);
}

void SetString(DynamicString& string, const char* src, size_t length) {
    if (string.capacity == 0) {
        string.capacity = 2;
    }

    if (string.data == nullptr) {
        string.data = new char[string.capacity];
    }

    if (string.capacity < std::strlen(src) + 1) {
        string.capacity = std::strlen(src) * 2;

        char* new_data = new char[string.capacity];

        delete[] string.data;
        string.data = new_data;
    }
    
    std::strncpy(string.data, src, length);
    string.data[length] = '\0';
    string.size = std::strlen(src);
}
