#include "datetime.hpp"

#include <charconv>
#include <cstring>
#include <iostream>

int8_t MonthToNumber(std::string_view month) {
    for (int i = 1; i <= 12; ++i) {
        if (month == kMonthsList[i - 1]) {
            return i;
        }
    }

    return -1;
}

bool IsLeapYear(int8_t year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int8_t GetDaysInMonth(int8_t month, int16_t year) {
    if (month == 2 && IsLeapYear(year)) {
        return 29;
    }

    return kDaysInMonth[month - 1];
}

uint64_t DateTimeToTimestamp(const DateTime& datetime) {
    uint64_t result = 0;

    for (int16_t year = 1970; year < datetime.year; ++year) {
        result += (IsLeapYear(year) ? 366 : 365) * 24 * 60 * 60;
    }

    for (int8_t month = 1; month < datetime.month; ++month) {
        result += GetDaysInMonth(month, datetime.year) * 24 * 60 * 60;
    }

    result += (datetime.day - 1) * 24 * 60 * 60;
    result += datetime.hours * 60 * 60;
    result += datetime.minutes * 60;
    result += datetime.seconds;

    return result;
}

int64_t ParseIntValue(std::string_view str) {
    int64_t result;
    std::from_chars_result convertion_result = std::from_chars(str.data(), str.data() + str.size(), result);

    if (convertion_result.ec == std::errc::invalid_argument || convertion_result.ptr != str.end()) {
        return -1;
    }

    return result;
}

uint64_t LocalTimeStringToTimestamp(std::string_view local_time) {
    DateTime datetime;

    // format: 01/Jul/1995:00:00:01 -0400

    if (local_time[2] != '/') {
        return 0;
    }


    int16_t day = ParseIntValue(local_time.substr(0, 2));
    local_time = local_time.substr(3);
    
    if (local_time[3] != '/') {
        return 0;
    }

    int16_t month = MonthToNumber(local_time.substr(0, 3));
    local_time = local_time.substr(4);

    if (local_time[4] != ':') {
        return 0;
    }

    int16_t year = ParseIntValue(local_time.substr(0, 4));
    local_time = local_time.substr(5);

    if (local_time[2] != ':') {
        return 0;
    }

    int16_t hours = ParseIntValue(local_time.substr(0, 2));
    local_time = local_time.substr(3);

    if (local_time[2] != ':') {
        return 0;
    }

    int16_t minutes = ParseIntValue(local_time.substr(0, 2));
    local_time = local_time.substr(3);

    if (local_time[2] != ' ') {
        return 0;
    }

    int16_t seconds = ParseIntValue(local_time.substr(0, 2));
    std::string_view timezone = local_time.substr(3);

    if (timezone.length() != 5 || (timezone[0] != '+' && timezone[0] != '-')) {
        return 0;
    }

    if (day == -1 || month == -1 || year == -1 || hours == -1 || minutes == -1 || seconds == -1) {
        return 0;
    }

    datetime.day = day;
    datetime.month = month;
    datetime.year = year;
    datetime.hours = hours;
    datetime.minutes = minutes;
    datetime.seconds = seconds;

    int8_t hours_shift = ParseIntValue(timezone.substr(1, 2));
    int8_t minutes_shift = ParseIntValue(timezone.substr(3, 2));

    if (hours_shift == -1 || minutes_shift == -1) {
        return 0;
    }

    int32_t seconds_shift = (hours_shift * 60 * 60) + (minutes_shift * 60); 
    if (timezone[0] == '+') {
        return DateTimeToTimestamp(datetime) - seconds_shift;
    }
    
    return DateTimeToTimestamp(datetime) + seconds_shift;
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

    return;
}

DateTime TimestampToDateTime(uint64_t timestamp) {
    DateTime datetime;

    uint32_t seconds_in_current_day = timestamp % (24 * 60 * 60);

    datetime.seconds = seconds_in_current_day % 60;
    datetime.minutes = (seconds_in_current_day / 60) % 60;
    datetime.hours = seconds_in_current_day / (60 * 60);

    int32_t days_left = (timestamp - seconds_in_current_day) / (60 * 60 * 24);

    uint16_t year;
    for (year = 1970; days_left >= 365;) {
        ++year;
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
