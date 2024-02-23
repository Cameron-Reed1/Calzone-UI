#include <time.h>
#include <vector>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include <lvgl.h>

#include "date_utils.h"
#include "events.h"


#define MAX_FILE_NAME_LEN 42
#define MAX_LINE_LEN 200
#define FILE_PATH_FMT "/var/lib/calzone/%02d_%d.csv"
#define FILE_LINE_FMT "%d,%d,%d,%d,%d,%u,%" STR_MAX_NAME_LEN "[^\t\n]"
#define FILE_LINE_FMT_QUOTE "%d,%d,%d,%d,%d,%u,\"%" STR_MAX_NAME_LEN "[^\t\n\"]\""


namespace Events {

static time_t modified_time = 0;
static event_type_counts event_counts[MAX_DAYS_IN_MONTH];
static lv_obj_t* objects_to_update[MAX_DAYS_IN_MONTH];
static lv_obj_t* detailed_events_object = nullptr;
static std::vector<Event> events;


void clearEvents(void);
void dispatchCallbacks(void);
bool wasModified(const char* const file_name);
int readEventsFromFile(const char* const file_name, int today);


void registerCallback(uint8_t day, lv_obj_t* object)
{
    day -= 1;
    if (day >= MAX_DAYS_IN_MONTH) {
        return;
    }

    objects_to_update[day] = object;
}

void registerDetailedCallback(lv_obj_t* object)
{
    detailed_events_object = object;
}

void unregisterCallbacks(void)
{
    for (size_t i = 0; i < MAX_DAYS_IN_MONTH; i++) {
        objects_to_update[i] = NULL;
    }
}

void update(uint8_t today, int month, uint32_t year)
{
    // No real reason to have the limit at year 100 000 000 since the pi zero's time will die in 2038,
    // but I just pushed nine a bunch of times and now this is how it is
    if (today < 1 || today > MAX_DAYS_IN_MONTH || month > 11 || year > 99999999) {
        return;
    }

    char file_name[MAX_FILE_NAME_LEN];
    sprintf(file_name, FILE_PATH_FMT, month + 1, year);

    // Check if file exists
    if (access(file_name, F_OK) != 0) {
        clearEvents();
        dispatchCallbacks();
        return;
    }

    if (wasModified(file_name)) {
        clearEvents();
        if (readEventsFromFile(file_name, today) != 0) {
            dispatchCallbacks();
        }
    }
}

void dispatchCallbacks(void)
{
    for (size_t i = 0; i < MAX_DAYS_IN_MONTH; i++) {
        if (objects_to_update[i] != nullptr) {
            lv_obj_send_event(objects_to_update[i], LV_EVENT_REFRESH, &event_counts[i]);
        }
    }

    if (detailed_events_object != nullptr) {
        lv_obj_send_event(detailed_events_object, LV_EVENT_REFRESH, &events);
    }
}

bool wasModified(const char* const file_name)
{
    struct stat file_stats;

    if (stat(file_name, &file_stats) != 0) {
        printf("Failed to acces file %s\n", file_name);
        return false;
    }

    return file_stats.st_mtim.tv_sec > modified_time;
}

void clearEvents(void)
{
    events.clear();
    modified_time = 0;

    for (size_t i = 0; i < MAX_DAYS_IN_MONTH; i++) {
        for (size_t j = 0; j < EVENTS_COUNT; j++) {
            event_counts[i].counts[j] = 0;
        }
    }
}

int readEventsFromFile(const char* const file_name, int today)
{
    FILE* f = fopen(file_name, "r");
    if (f == nullptr) {
        printf("Failed to acces file %s\n", file_name);
        return 0;
    }


    char buf[MAX_LINE_LEN];

    int day;
    int num_events = 0;
    Event event;

    while (1) {
        char* b = fgets(buf, MAX_LINE_LEN, f);
        if (b == nullptr) {
            // Error or EOF
            break;
        }

        if (buf[0] == '\n') {
            // Empty line
            continue;
        }

        unsigned int event_type_int;
        int num = sscanf(buf, FILE_LINE_FMT_QUOTE,
                &event.s_hour, &event.s_min, &event.e_hour, &event.e_min, &day, &event_type_int, event.name);
        if (num != 7) {
            num = sscanf(buf, FILE_LINE_FMT,
                &event.s_hour, &event.s_min, &event.e_hour, &event.e_min, &day, &event_type_int, event.name);
            if (num != 7) {
                printf("Malformed line in events list file\n");
                continue;
            }
        }

        if (day < 1 || day > MAX_DAYS_IN_MONTH) {
            printf("Invalid day %d\n", day);
        }

        event.type = static_cast<EVENT_TYPE>(event_type_int);
        if (event.type >= EVENTS_COUNT) {
            event.type = EVENTS_OTHER;
        }

        if (day == today) {
            events.push_back(event);
        }
        
        event_counts[day - 1].counts[event.type] += 1;
        num_events++;
    }

    fclose(f);

    return num_events;
}

} // namespace Events

