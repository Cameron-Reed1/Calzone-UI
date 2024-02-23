#pragma once

#include <stdint.h>

#include <lvgl.h>

#include "date_utils.h"


#define MAX_NAME_LEN 100
#define STR_MAX_NAME_LEN "99"


namespace Events {


enum EVENT_TYPE {
    EVENTS_MEETING = 0,
    EVENTS_CASCADE,
    EVENTS_SETUP,
    EVENTS_APPOINTMENT,
    EVENTS_OTHER,
    EVENTS_COUNT,
};

struct event_type_counts {
    uint8_t counts[EVENTS_COUNT];
//    uint8_t other;
//    uint8_t meetings;
//    uint8_t ram_upgrades;
//    uint8_t cascades;
//    uint8_t setups;
//    uint8_t appointments
};

struct Event {
    int s_hour;
    int s_min;
    int e_hour;
    int e_min;
    EVENT_TYPE type;
    char name[MAX_NAME_LEN];
};



void unregisterCallbacks(void);
void registerCallback(uint8_t day, lv_obj_t* object);
void registerDetailedCallback(lv_obj_t* object);
void update(uint8_t today, int month, uint32_t year);

} // namespace Events

