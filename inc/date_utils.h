#pragma once

#include <stdint.h>
#include <stdbool.h>


#define MAX_DAYS_IN_MONTH 31


bool isLeapYear(int year);
uint8_t firstDayOfMonth(int month, int year);
uint8_t daysInMonth(int month, int year);

