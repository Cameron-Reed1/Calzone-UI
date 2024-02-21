#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <lvgl.h>

#include "date_utils.h"
#include "calendar.h"
#include "fonts.h"


static lv_style_t box_style;
static lv_style_t box_style_active;
static lv_style_t box_style_inactive;

static const char* const month_list[] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
};

static const char* const event_format_strs[] = {
    NF_SYMBOL_CAMERA " %d meeting(s)",
    NF_SYMBOL_UPDATE " %d cascade(s)",
    NF_SYMBOL_LAPTOP " %d setup(s)",
    NF_SYMBOL_CALENDAR " %d appointment(s)",
    NF_SYMBOL_CALENDAR_QUESTION " %d other event(s)",
};


lvc_calendar_row_t* create_row(lv_obj_t* parent);
lvc_calendar_box_t* create_box(lv_obj_t* parent);
void destroy_row(lvc_calendar_row_t* row);
void destroy_box(lvc_calendar_box_t* row);
void style_calendar(lvc_calendar_t* cal, const struct tm* const ct);
int style_row(lvc_calendar_row_t* row, lv_obj_t* last_row, uint32_t box_width, uint32_t box_height, int day, int current_day, int days_in_month);
void style_box(lvc_calendar_box_t* box, uint32_t width, uint32_t height, int day, int wday, bool today, int days_in_month);
void draw_event_list_cb(lv_event_t* event);
void calendar_refresh_event(lv_event_t* event);



static inline void init_style(uint32_t box_width, uint32_t box_height)
{
    lv_style_init(&box_style);
    lv_style_set_width(&box_style, box_width);
    lv_style_set_height(&box_style, box_height);
    lv_style_set_radius(&box_style, 0);
    lv_style_set_bg_color(&box_style, lv_color_hex(0xffffff));
    lv_style_set_bg_opa(&box_style, LV_OPA_COVER);
    lv_style_set_border_width(&box_style, 2);
    lv_style_set_border_color(&box_style, lv_color_hex(0x333333));
    lv_style_set_pad_all(&box_style, 5);

    lv_style_init(&box_style_active);
    lv_style_set_bg_color(&box_style_active, lv_color_hex(0xbbbbbb));
    lv_style_set_text_color(&box_style_active, lv_color_hex(0x000000));

    lv_style_init(&box_style_inactive);
    lv_style_set_bg_color(&box_style_inactive, lv_color_hex(0x646464));
}

lvc_calendar_t* create_calendar(lv_obj_t* parent, uint32_t width)
{
    time_t t = time(NULL);
    struct tm* ct = localtime(&t);

    lvc_calendar_t* cal = malloc(sizeof(lvc_calendar_t));
    cal->calendar = lv_obj_create(parent);
    cal->title_bar = lv_obj_create(cal->calendar);
    cal->title_label = lv_label_create(cal->title_bar);
    cal->width = width - (width % 7);
    cal->box_width = cal->width / 7;
    cal->box_height = cal->box_width * 0.8;

    init_style(cal->box_width, cal->box_height);

    for (uint8_t i = 0; i < CALENDAR_ROWS; i++) {
        cal->rows[i] = create_row(cal->calendar);
    }

    style_calendar(cal, ct);

    lv_obj_add_event_cb(cal->calendar, calendar_refresh_event, LV_EVENT_REFRESH, cal);

    return cal;
}

lvc_calendar_row_t* create_row(lv_obj_t* parent)
{
    lvc_calendar_row_t* row = malloc(sizeof(lvc_calendar_row_t));

    for (uint8_t i = 0; i < DAYS_IN_WEEK; i++) {
        row->boxes[i] = create_box(parent);
    }

    return row;
}

lvc_calendar_box_t* create_box(lv_obj_t* parent)
{
    lvc_calendar_box_t* box = malloc(sizeof(lvc_calendar_box_t));
    
    box->box = lv_obj_create(parent);
    box->label = lv_label_create(box->box);

    for (size_t i = 0; i < EVENTS_COUNT; i++) {
        box->event_count_labels[i] = lv_label_create(box->box);
    }

    return box;
}

void destroy_calendar(lvc_calendar_t* cal)
{
    for (uint8_t i = 0; i < CALENDAR_ROWS; i++) {
        destroy_row(cal->rows[i]);
    }

    lv_obj_delete(cal->title_label);
    lv_obj_delete(cal->title_bar);
    lv_obj_delete(cal->calendar);

    free(cal);
}

void destroy_row(lvc_calendar_row_t* row)
{
    for (uint8_t i = 0; i < DAYS_IN_WEEK; i++) {
        destroy_box(row->boxes[i]);
    }

    free(row);
}

void destroy_box(lvc_calendar_box_t* box)
{
    lv_obj_delete(box->label);

    for (size_t i = 0; i < EVENTS_COUNT; i++) {
        lv_obj_delete(box->event_count_labels[i]);
    }

    lv_obj_delete(box->box);

    free(box);
}

void style_calendar(lvc_calendar_t* cal, const struct tm* const ct)
{
    int days = daysInMonth(ct->tm_mon, ct->tm_year + 1900);
    int start_day = 1 - firstDayOfMonth(ct->tm_mon, ct->tm_year + 1900);

    if (start_day == -5 && days == 31) {
        start_day += 7;
    }
    
    lv_obj_set_size(cal->calendar, cal->width, cal->box_height * 6);
    lv_obj_set_style_pad_all(cal->calendar, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(cal->calendar, 0, LV_PART_MAIN);

    lv_obj_remove_style_all(cal->title_bar);
    lv_obj_set_size(cal->title_bar, cal->width, cal->box_height);
    lv_obj_align(cal->title_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_pad_all(cal->title_bar, 0, LV_PART_MAIN);
    lv_obj_add_style(cal->title_bar, &box_style, LV_PART_MAIN);

    lv_obj_remove_style_all(cal->title_label);
    lv_label_set_text_fmt(cal->title_label, "%s %d", month_list[ct->tm_mon], ct->tm_year + 1900);
    lv_obj_set_style_text_font(cal->title_label, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_center(cal->title_label);

    unregisterAllUpdates();

    lv_obj_t* last_row = cal->title_bar;
    for (uint8_t r = 0; r < CALENDAR_ROWS; r++) {
        start_day = style_row(cal->rows[r], last_row, cal->box_width, cal->box_height, start_day, ct->tm_mday, days);
        last_row = cal->rows[r]->boxes[0]->box;
    }

    // Update events, so it will call our callbacks set in style_box to show the list of events
    updateEvents(ct->tm_mday, ct->tm_mon, ct->tm_year + 1900);
}

int style_row(lvc_calendar_row_t* row, lv_obj_t* last_row, uint32_t box_width, uint32_t box_height, int day, int current_day, int days_in_month)
{
    style_box(row->boxes[0], box_width, box_height, day, 0, day == current_day, days_in_month);
    lv_obj_align_to(row->boxes[0]->box, last_row, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    day++;

    lv_obj_t* last = row->boxes[0]->box;
    for (uint8_t i = 1; i < DAYS_IN_WEEK; i++) {
        style_box(row->boxes[i], box_width, box_height, day, i, day == current_day, days_in_month);
        lv_obj_align_to(row->boxes[i]->box, last, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
        last = row->boxes[i]->box;
        day++;
    }

    return day;
}

void style_box(lvc_calendar_box_t* box, uint32_t width, uint32_t height, int day, int wday, bool today, int days_in_month)
{
    lv_obj_remove_style_all(box->box);
    lv_obj_set_size(box->box, width, height);

    lv_obj_add_style(box->box, &box_style, LV_PART_MAIN);
    if (today) {
        lv_obj_add_style(box->box, &box_style_active, LV_PART_MAIN);
    }
        
    if (wday == 6) {
        lv_obj_set_style_border_side(box->box, LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_RIGHT, LV_PART_MAIN);
    } else {
        lv_obj_set_style_border_side(box->box, LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_LEFT, LV_PART_MAIN);
    }


    lv_obj_align(box->label, LV_ALIGN_TOP_LEFT, 0, 0);

    if (day > 0 && day <= days_in_month) {
        lv_label_set_text_fmt(box->label, "%d", day);
        lv_obj_set_style_pad_bottom(box->label, 8, LV_PART_MAIN);
        lv_obj_add_event_cb(box->box, draw_event_list_cb, LV_EVENT_REFRESH, box);
        registerUpdateOnEventChange(day, box->box);
    } else {
        lv_obj_add_style(box->box, &box_style_inactive, LV_PART_MAIN);
        lv_label_set_text(box->label, "");
    }

    for (size_t i = 0; i < EVENTS_COUNT; i++) {
        lv_label_set_text(box->event_count_labels[i], "");
    }
}


void draw_event_list_cb(lv_event_t* event)
{
    event_type_counts_t* counts = lv_event_get_param(event);
    lvc_calendar_box_t* box = lv_event_get_user_data(event);

    int c = 0;
    lv_obj_t* label = NULL;
    lv_obj_t* last = box->label;

    for (size_t i = 0; i < EVENTS_COUNT; i++) {
        c = counts->counts[i];
        label = box->event_count_labels[i];

        if (c != 0) {
            lv_label_set_text_fmt(label, event_format_strs[i], c);
            lv_obj_align_to(label, last, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
            lv_obj_set_style_text_font(label, &nerdfonts_arimo_14, LV_PART_MAIN);
            last = label;
        } else {
            lv_label_set_text(label, "");
        }
    }
}


void calendar_refresh_event(lv_event_t* event)
{
	struct tm* ct = lv_event_get_param(event);
    lvc_calendar_t* cal = lv_event_get_user_data(event);

    style_calendar(cal, ct);
}

