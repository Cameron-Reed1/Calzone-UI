#pragma once

#include <stdint.h>
#include <stdbool.h>


#define MAX_DAYS_IN_MONTH 31


typedef enum {
    JANUARY = 0,
    FEBRUARY,
    MARCH,
    APRIL,
    MAY,
    JUNE,
    JULY,
    AUGUST,
    SEPTEMBER,
    OCTOBER,
    NOVEMBER,
    DECEMBER,
} MONTH;


bool isLeapYear(int year);
uint8_t firstDayOfMonth(MONTH month, int year);
uint8_t daysInMonth(MONTH month, int year);

