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

Calendar::Calendar(lv_obj_t* parent, uint32_t width)
    : width(width - (width % 7))
{
    time_t t = time(NULL);
    struct tm* ct = localtime(&t);

    calendar = lv_obj_create(parent);
    title_bar = lv_obj_create(calendar);
    title_label = lv_label_create(title_bar);
    box_width = width / 7;
    box_height = box_width * 0.8;

    init_style(box_width, box_height);

    for (uint8_t i = 0; i < CALENDAR_ROWS; i++) {
        rows[i] = new CalendarRow(calendar);
    }

    Style(ct);

    lv_obj_add_event_cb(calendar, calendar_refresh_event, LV_EVENT_REFRESH, this);
}

CalendarRow::CalendarRow(lv_obj_t* parent)
{
    for (uint8_t i = 0; i < DAYS_IN_WEEK; i++) {
        boxes[i] = new CalendarBox(parent);
    }
}

CalendarBox::CalendarBox(lv_obj_t* parent)
{
    box = lv_obj_create(parent);
    label = lv_label_create(box);

    for (size_t i = 0; i < Events::EVENTS_COUNT; i++) {
        event_count_labels[i] = lv_label_create(box);
    }
}

Calendar::~Calendar()
{
    for (uint8_t i = 0; i < CALENDAR_ROWS; i++) {
        delete rows[i];
    }

    lv_obj_delete(title_label);
    lv_obj_delete(title_bar);
    lv_obj_delete(calendar);
}

CalendarRow::~CalendarRow()
{
    for (uint8_t i = 0; i < DAYS_IN_WEEK; i++) {
        delete boxes[i];
    }
}

CalendarBox::~CalendarBox()
{
    lv_obj_delete(label);

    for (size_t i = 0; i < Events::EVENTS_COUNT; i++) {
        lv_obj_delete(event_count_labels[i]);
    }

    lv_obj_delete(box);
}

void Calendar::Style(const struct tm* const ct)
{
    int days = daysInMonth(ct->tm_mon, ct->tm_year + 1900);
    int start_day = 1 - firstDayOfMonth(ct->tm_mon, ct->tm_year + 1900);

    if (start_day == -5 && days == 31) {
        start_day += 7;
    }
    
    lv_obj_set_size(calendar, width, box_height * 6);
    lv_obj_set_style_pad_all(calendar, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(calendar, 0, LV_PART_MAIN);

    lv_obj_remove_style_all(title_bar);
    lv_obj_set_size(title_bar, width, box_height);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_pad_all(title_bar, 0, LV_PART_MAIN);
    lv_obj_add_style(title_bar, &box_style, LV_PART_MAIN);

    lv_obj_remove_style_all(title_label);
    lv_label_set_text_fmt(title_label, "%s %d", month_list[ct->tm_mon], ct->tm_year + 1900);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_center(title_label);

    Events::unregisterCallbacks();

    lv_obj_t* last_row = title_bar;
    for (uint8_t r = 0; r < CALENDAR_ROWS; r++) {
        start_day = rows[r]->Style(last_row, box_width, box_height, start_day, ct->tm_mday, days);
        last_row = rows[r]->boxes[0]->box;
    }

    // Update events, so it will call our callbacks set in style_box to show the list of events
    Events::update(ct->tm_mday, ct->tm_mon, ct->tm_year + 1900);
}

int CalendarRow::Style(lv_obj_t* last_row, uint32_t box_width, uint32_t box_height, int day, int current_day, int days_in_month)
{
    boxes[0]->Style(box_width, box_height, day, 0, day == current_day, days_in_month);
    lv_obj_align_to(boxes[0]->box, last_row, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    day++;

    lv_obj_t* last = boxes[0]->box;
    for (uint8_t i = 1; i < DAYS_IN_WEEK; i++) {
        boxes[i]->Style(box_width, box_height, day, i, day == current_day, days_in_month);
        lv_obj_align_to(boxes[i]->box, last, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
        last = boxes[i]->box;
        day++;
    }

    return day;
}

void CalendarBox::Style(uint32_t width, uint32_t height, int day, int wday, bool today, int days_in_month)
{
    lv_obj_remove_style_all(box);
    lv_obj_set_size(box, width, height);

    lv_obj_add_style(box, &box_style, LV_PART_MAIN);
    if (today) {
        lv_obj_add_style(box, &box_style_active, LV_PART_MAIN);
    }
        
    if (wday == 6) {
        lv_obj_set_style_border_side(box, LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_RIGHT, LV_PART_MAIN);
    } else {
        lv_obj_set_style_border_side(box, LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_LEFT, LV_PART_MAIN);
    }


    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

    if (day > 0 && day <= days_in_month) {
        lv_label_set_text_fmt(label, "%d", day);
        lv_obj_set_style_pad_bottom(label, 8, LV_PART_MAIN);
        lv_obj_add_event_cb(box, CalendarBox::DrawEvents_cb, LV_EVENT_REFRESH, this);
        Events::registerCallback(day, box);
    } else {
        lv_obj_add_style(box, &box_style_inactive, LV_PART_MAIN);
        lv_label_set_text(label, "");
    }

    for (size_t i = 0; i < Events::EVENTS_COUNT; i++) {
        lv_label_set_text(event_count_labels[i], "");
    }
}


void CalendarBox::DrawEvents_cb(lv_event_t* event)
{
    Events::event_type_counts* counts = (Events::event_type_counts*) lv_event_get_param(event);
    CalendarBox* box = (CalendarBox*) lv_event_get_user_data(event);

    int c = 0;
    lv_obj_t* label = NULL;
    lv_obj_t* last = box->label;

    for (size_t i = 0; i < Events::EVENTS_COUNT; i++) {
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
	struct tm* ct = (struct tm*) lv_event_get_param(event);
    Calendar* cal = (Calendar*) lv_event_get_user_data(event);

    cal->Style(ct);
}

