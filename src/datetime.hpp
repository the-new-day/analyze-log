#pragma once

#include <cstdint>
#include <string_view>
#include <optional>

constexpr const char* kMonthsList[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
constexpr int8_t kDaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

struct DateTime {
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
};

std::optional<uint8_t> MonthToNumber(std::string_view month);

std::optional<uint64_t> DateTimeToTimestamp(const DateTime& datetime);

std::optional<uint64_t> LocalTimeStringToTimestamp(std::string_view local_time);

DateTime TimestampToDateTime(uint64_t timestamp);

void TimestampToDateTimeString(uint64_t timestamp, char buffer[27]);

bool IsLeapYear(uint8_t year);

std::optional<uint8_t> GetDaysInMonth(uint8_t month, uint16_t year);
