#include <stdint.h>
#include <stdbool.h>

#include "date_utils.h"



static const int century_start[]      = { 0, 6, 4, 2 };
static const int offsets[]            = { 0,  3,  3,  6,  1,  4,  6,  2,  5,  0,  3,  5  };
static const int leap_offsets[]       = { 0,  3,  4,  0,  2,  5,  0,  3,  6,  1,  4,  6  };
static const int days_in_month[]      = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static const int leap_days_in_month[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };



bool isLeapYear(int year)
{
    return (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0);
}

uint8_t firstDayOfMonth(MONTH month, int year)
{
    int d = year % 100;
    uint8_t first_of_year = (century_start[(year / 100) % 4] + d + (d / 4) - isLeapYear(year) + 7) % 7;

    if (isLeapYear(year)) {
        return (first_of_year + leap_offsets[month]) % 7;
    }
    return (first_of_year + offsets[month]) % 7;
}

uint8_t daysInMonth(MONTH month, int year)
{
    if (isLeapYear(year))
    {
        return leap_days_in_month[month];
    }
    return days_in_month[month];
}

