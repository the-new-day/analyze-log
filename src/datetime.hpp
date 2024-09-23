#pragma once

#include <cstdint>
#include <cstring>
#include <string_view>

constexpr std::string_view kMonthsList[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
constexpr int8_t kDaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

struct DateTime {
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
};

int8_t MonthToNumber(std::string_view month);

uint64_t DateTimeToTimestamp(const DateTime& datetime);

uint64_t LocalTimeStringToTimestamp(std::string_view local_time);
