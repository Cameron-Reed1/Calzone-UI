#pragma once

#include <cstdint>
#include <stdint.h>

#include <lvgl.h>

#include "events.h"


#define DAYS_IN_WEEK 7
#define CALENDAR_ROWS 5


struct CalendarBox {
public:
    CalendarBox(lv_obj_t* parent);
    ~CalendarBox();

    void Style(uint32_t width, uint32_t height, int day, int wday, bool today, int days_in_month);

public:
    lv_obj_t* box;

private:
    static void DrawEvents_cb(lv_event_t* event);

private:
    lv_obj_t* label;

    lv_obj_t* event_count_labels[Events::EVENTS_COUNT];
};


struct CalendarRow {
public:
    CalendarRow(lv_obj_t* parent);
    ~CalendarRow();

    int Style(lv_obj_t* last_row, uint32_t box_width, uint32_t box_height, int day, int current_day, int days_in_month);

public:
    CalendarBox* boxes[DAYS_IN_WEEK];
};


class Calendar {
public:
    Calendar(lv_obj_t* parent, uint32_t width);
    ~Calendar();

    void Style(const struct tm* const ct);

public:
    lv_obj_t* calendar;

private:
    lv_obj_t* title_bar;
    lv_obj_t* title_label;
    CalendarRow* rows[CALENDAR_ROWS];
    
    uint32_t width;
    uint32_t box_width;
    uint32_t box_height;
};

