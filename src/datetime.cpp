#include "datetime.hpp"
#include "argparsing.hpp"

#include <charconv>
#include <cstring>
#include <iostream>
#include <expected>

std::optional<uint8_t> MonthToNumber(std::string_view month) {
    for (int i = 1; i <= 12; ++i) {
        if (month == kMonthsList[i - 1]) {
            return i;
        }
    }

    return std::nullopt;
}

bool IsLeapYear(uint16_t year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

std::optional<uint8_t> GetDaysInMonth(uint8_t month, uint16_t year) {
    if (month == 2 && IsLeapYear(year)) {
        return 29;
    }

    if (month > 12) {
        return std::nullopt;
    }

    return kDaysInMonth[month - 1];
}

std::optional<uint64_t> DateTimeToTimestamp(const DateTime& datetime) {
    uint64_t result = 0;

    for (uint16_t year = 1970; year < datetime.year; ++year) {
        result += (IsLeapYear(year) ? 366 : 365) * 24 * 60 * 60;
    }

    for (int8_t month = 1; month < datetime.month; ++month) {
        std::optional<uint8_t> days_in_month = GetDaysInMonth(month, datetime.year);
        if (!days_in_month.has_value()) {
            return std::nullopt;
        }

        result += days_in_month.value() * 24 * 60 * 60;
    }

    result += (datetime.day - 1) * 24 * 60 * 60;
    result += datetime.hours * 60 * 60;
    result += datetime.minutes * 60;
    result += datetime.seconds;

    return result;
}

std::optional<uint64_t> LocalTimeStringToTimestamp(std::string_view local_time) {
    DateTime datetime;

    // format: 01/Jul/1995:00:00:01 -0400

    if (local_time[2] != '/') {
        return std::nullopt;
    }

    std::expected<int64_t, const char*> day = ParseInt(local_time.substr(0, 2));
    if (!day.has_value()) {
        return std::nullopt;
    }

    local_time = local_time.substr(3);
    
    if (local_time[3] != '/') {
        return std::nullopt;
    }

    std::optional<uint8_t> month = MonthToNumber(local_time.substr(0, 3));
    if (!month.has_value()) {
        return std::nullopt;
    }

    local_time = local_time.substr(4);

    if (local_time[4] != ':') {
        return std::nullopt;
    }

    std::expected<int64_t, const char*> year = ParseInt(local_time.substr(0, 4));
    if (!year.has_value()) {
        return std::nullopt;
    }

    local_time = local_time.substr(5);

    if (local_time[2] != ':') {
        return std::nullopt;
    }

    std::expected<int64_t, const char*> hours = ParseInt(local_time.substr(0, 2));
    if (!hours.has_value()) {
        return std::nullopt;
    }

    local_time = local_time.substr(3);

    if (local_time[2] != ':') {
        return std::nullopt;
    }

    std::expected<int64_t, const char*> minutes = ParseInt(local_time.substr(0, 2));
    if (!minutes.has_value()) {
        return std::nullopt;
    }

    local_time = local_time.substr(3);

    if (local_time[2] != ' ') {
        return std::nullopt;
    }

    std::expected<int64_t, const char*> seconds = ParseInt(local_time.substr(0, 2));
    if (!seconds.has_value()) {
        return std::nullopt;
    }

    std::string_view timezone = local_time.substr(3);

    if (timezone.length() != 5 || (timezone[0] != '+' && timezone[0] != '-')) {
        return std::nullopt;
    }

    if (day.value() == -1 
     || month.value() == -1 
     || year.value() == -1 
     || hours.value() == -1 
     || minutes.value() == -1 
     || seconds.value() == -1)
    {
        return std::nullopt;
    }

    datetime.day = day.value();
    datetime.month = month.value();
    datetime.year = year.value();
    datetime.hours = hours.value();
    datetime.minutes = minutes.value();
    datetime.seconds = seconds.value();

    std::expected<int64_t, const char*> hours_shift = ParseInt(timezone.substr(1, 2));
    if (!hours_shift.has_value()) {
        return std::nullopt;
    }

    std::expected<int64_t, const char*> minutes_shift = ParseInt(timezone.substr(3, 2));
    if (!minutes_shift.has_value()) {
        return std::nullopt;
    }

    uint32_t seconds_shift = (hours_shift.value() * 60 * 60) + (minutes_shift.value() * 60);
    std::optional<uint64_t> result = DateTimeToTimestamp(datetime);
    if (!result.has_value()) {
        return std::nullopt;
    }

    if (timezone[0] == '+') {
        return result.value() - seconds_shift;
    }
    
    return result.value() + seconds_shift;
}

void Convert2DigitNumberToString(uint8_t number, char buffer[3]) {
    if (number < 10) {
        std::snprintf(buffer, 3, "0%d", number);
    } else {
        std::snprintf(buffer, 3, "%d", number);
    }
}

void TimestampToDateTimeString(uint64_t timestamp, char buffer[27]) {
    DateTime datetime = TimestampToDateTime(timestamp);

    // format: 01/Jul/1995:00:00:01 -0400 (length: 26 + '\0')
    char day[3];
    char month[4];
    char year[6];
    char hours[3];
    char minutes[3];
    char seconds[3];
    
    Convert2DigitNumberToString(datetime.day, day);
    Convert2DigitNumberToString(datetime.hours, hours);
    Convert2DigitNumberToString(datetime.minutes, minutes);
    Convert2DigitNumberToString(datetime.seconds, seconds);

    std::sprintf(year, "%d", datetime.year);

    std::sprintf(buffer, "%2s/%3s/%4s:%2s:%2s:%2s +0000", day, kMonthsList[datetime.month - 1], year, hours, minutes, seconds);
}

DateTime TimestampToDateTime(uint64_t timestamp) {
    DateTime datetime;

    uint32_t seconds_in_current_day = timestamp % (24 * 60 * 60);

    datetime.seconds = seconds_in_current_day % 60;
    datetime.minutes = (seconds_in_current_day / 60) % 60;
    datetime.hours = seconds_in_current_day / (60 * 60);

    int32_t days_left = (timestamp - seconds_in_current_day) / (60 * 60 * 24);

    uint16_t year;
    for (year = 1970; days_left >= (IsLeapYear(year) ? 366 : 365); ++year) {
        days_left -= (IsLeapYear(year) ? 366 : 365);
    }

    datetime.year = year;

    if (IsLeapYear(year)) {
        ++days_left;
    }

    ++days_left;

    uint8_t month = 0;
    while (days_left > 0) {
        if (month == 1 && IsLeapYear(year)) {
            days_left -= 29;
        } else {
            days_left -= kDaysInMonth[month];
        }

        ++month;
    }
    
    datetime.month = month;

    if (month == 2 && IsLeapYear(year)) {
        datetime.day = days_left + 29;
    } else {
        datetime.day = days_left + kDaysInMonth[month - 1];
    }

    return datetime;
}
