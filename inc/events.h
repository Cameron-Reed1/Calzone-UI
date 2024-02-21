#pragma once

#include <stdint.h>

#include <lvgl.h>

#include "date_utils.h"


#define MAX_NAME_LEN 100
#define STR_MAX_NAME_LEN "99"


typedef enum {
    EVENTS_MEETING = 0,
    EVENTS_CASCADE,
    EVENTS_SETUP,
    EVENTS_APPOINTMENT,
    EVENTS_OTHER,
    EVENTS_COUNT,
} EVENT_TYPE;

typedef struct {
    uint8_t counts[EVENTS_COUNT];
//    uint8_t other;
//    uint8_t meetings;
//    uint8_t ram_upgrades;
//    uint8_t cascades;
//    uint8_t setups;
//    uint8_t appointments
} event_type_counts_t;

typedef struct {
    int s_hour;
    int s_min;
    int e_hour;
    int e_min;
    EVENT_TYPE type;
    char name[MAX_NAME_LEN];
} event_t;

typedef struct {
    event_t* events;
    size_t size;
    size_t capacity;
} events_t;



void unregisterAllUpdates(void);
void registerUpdateOnEventChange(uint8_t day, lv_obj_t* object);
void register_update_on_detailed_events_change(lv_obj_t* object);
void updateEvents(uint8_t today, MONTH month, uint32_t year);

