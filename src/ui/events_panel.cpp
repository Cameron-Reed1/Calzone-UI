#include <cstddef>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <time.h>

#include <lvgl.h>
#include <vector>

#include "events.h"
#include "events_panel.h"


void setEventLabelText(lv_obj_t* label, Events::Event* event);


EventsPanel::EventsPanel(lv_obj_t* parent)
{
    panel = lv_obj_create(parent);
    header = lv_label_create(panel);

    lv_obj_set_style_pad_all(panel, 10, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
    
    lv_label_set_text(header, "Events");
    lv_obj_set_style_text_font(header, &lv_font_montserrat_36, LV_PART_MAIN);
    lv_obj_align(header, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_add_event_cb(panel, EventsPanel::DrawEventList_cb, LV_EVENT_REFRESH, this);

    Events::registerDetailedCallback(panel);
}

EventsPanel::~EventsPanel()
{
    lv_obj_delete(header);

    for (size_t i = 0; i < event_labels.size(); i++) {
        lv_obj_delete(event_labels[i]);
    }

    lv_obj_delete(panel);
}

void EventsPanel::DrawEventList_cb(lv_event_t* event)
{
    EventsPanel* panel = (EventsPanel*) lv_event_get_user_data(event);
    std::vector<Events::Event>* events = (std::vector<Events::Event>*) lv_event_get_param(event);

    if (panel->event_labels.size() < events->size()) {
        panel->event_labels.reserve(events->size());

        while (panel->event_labels.size() < events->size()) {
            panel->event_labels.push_back(lv_label_create(panel->panel));
        }
    }


    lv_obj_t* last = panel->header;
    size_t i = 0;
    for (; i < events->size(); i++) {
        setEventLabelText(panel->event_labels[i], &events->at(i));
        lv_obj_set_style_text_font(panel->event_labels[i], &lv_font_montserrat_18, LV_PART_MAIN);
        lv_obj_align_to(panel->event_labels[i], last, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
        last = panel->event_labels[i];
    }


    if (events->size() < panel->event_labels.size()) {
        for (size_t j = events->size(); j < panel->event_labels.size(); j++) {
            lv_obj_delete(panel->event_labels[j]);
        }
        
        panel->event_labels.erase(std::next(panel->event_labels.begin(), i), panel->event_labels.end());
    }
}

void setEventLabelText(lv_obj_t* label, Events::Event* event)
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

