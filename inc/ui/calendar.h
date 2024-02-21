#pragma once

#include <stdint.h>

#include <lvgl.h>

#include "events.h"


#define DAYS_IN_WEEK 7
#define CALENDAR_ROWS 5


typedef struct lvc_calendar_box {
    lv_obj_t* box;
    lv_obj_t* label;

    lv_obj_t* event_count_labels[EVENTS_COUNT];
} lvc_calendar_box_t;

typedef struct lvc_calendar_row {
    lvc_calendar_box_t* boxes[DAYS_IN_WEEK];
} lvc_calendar_row_t;

typedef struct lvc_calendar {
    lv_obj_t* calendar;
    lv_obj_t* title_bar;
    lv_obj_t* title_label;
    lvc_calendar_row_t* rows[CALENDAR_ROWS];
    
    uint32_t width;
    uint32_t box_width;
    uint32_t box_height;

} lvc_calendar_t;



lvc_calendar_t* create_calendar(lv_obj_t* parent, uint32_t width);
void destroy_calendar(lvc_calendar_t* cal);
