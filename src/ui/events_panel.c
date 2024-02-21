#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <lvgl.h>

#include "events.h"
#include "events_panel.h"


int add_event_labels(lvc_events_panel_t* panel, unsigned int count);
void draw_events_list_cb(lv_event_t* event);
void event_label_set_text(lv_obj_t* label, event_t* event);


lvc_events_panel_t* create_events_panel(lv_obj_t* parent)
{
    lvc_events_panel_t* panel = malloc(sizeof(lvc_events_panel_t));

    panel->panel = lv_obj_create(parent);
    panel->header = lv_label_create(panel->panel);
    panel->event_labels = NULL;
    panel->event_labels_count = 0;

    lv_obj_set_style_pad_all(panel->panel, 10, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(panel->panel, LV_SCROLLBAR_MODE_OFF);
    
    lv_label_set_text(panel->header, "Events");
    lv_obj_set_style_text_font(panel->header, &lv_font_montserrat_36, LV_PART_MAIN);
    lv_obj_align(panel->header, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_add_event_cb(panel->panel, draw_events_list_cb, LV_EVENT_REFRESH, panel);

    register_update_on_detailed_events_change(panel->panel);

    return panel;
}

void destroy_events_panel(lvc_events_panel_t* panel)
{
    lv_obj_delete(panel->header);

    for (size_t i = 0; i < panel->event_labels_count; i++) {
        lv_obj_delete(panel->event_labels[i]);
    }

    lv_obj_delete(panel->panel);

    free(panel);
}

int add_event_labels(lvc_events_panel_t* panel, unsigned int count)
{
    uint16_t newCapacity = panel->event_labels_count + count;
    lv_obj_t** newMem = reallocarray(panel->event_labels, newCapacity, sizeof(lv_obj_t*));
    if (newMem == NULL) {
        printf("Call to reallocarray failed. Unable to create more labels to display events");
        return -1;
    }

    panel->event_labels = newMem;
    for (size_t i = panel->event_labels_count; i < newCapacity; i++) {
        panel->event_labels[i] = lv_label_create(panel->panel);
    }
    panel->event_labels_count = newCapacity;
    return 0;
}

void draw_events_list_cb(lv_event_t* event)
{
    lvc_events_panel_t* panel = lv_event_get_user_data(event);
    events_t* events = lv_event_get_param(event);

    if (panel->event_labels_count < events->size) {
        // More events than labels. Create new labels for events
        if (add_event_labels(panel, events->size - panel->event_labels_count) != 0) {
            printf("Failed to create new labels for event panel\n");
            return;
        }
    }

    lv_obj_t* last = panel->header;
    size_t i = 0;
    for (; i < events->size; i++) {
        event_label_set_text(panel->event_labels[i], &events->events[i]);
        lv_obj_set_style_text_font(panel->event_labels[i], &lv_font_montserrat_18, LV_PART_MAIN);
        lv_obj_align_to(panel->event_labels[i], last, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
        last = panel->event_labels[i];
    }

    const uint16_t len = panel->event_labels_count;
    for (;i < len; i++) {
        lv_obj_delete(panel->event_labels[i]);
        panel->event_labels_count -= 1;
    }
}

void event_label_set_text(lv_obj_t* label, event_t* event)
{
    int s_hour = event->s_hour;
    const char* const s_time_suffix = s_hour >= 12 ? "PM" : "AM";

    if (s_hour > 12) {
        s_hour -= 12;
    }

    if (event->e_hour < 0) {
        lv_label_set_text_fmt(label, "%s\n%d:%02d %s",
                event->name, s_hour, event->s_min, s_time_suffix);
    } else {
        int e_hour = event->e_hour;
        const char* const e_time_suffix = e_hour >= 12 ? "PM" : "AM";
    
        if (e_hour > 12) {
            e_hour -= 12;
        }

        lv_label_set_text_fmt(label, "%s\n%d:%02d %s - %d:%02d %s",
                event->name, s_hour, event->s_min, s_time_suffix, e_hour, event->e_min, e_time_suffix);
    }
}

