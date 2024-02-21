#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include <lvgl.h>

#include "lvgl_port/lvgl_fb_port.h"
#include "events_panel.h"
#include "calendar.h"
#include "events.h"


#define TIME_BUF_LEN 12


static lv_obj_t* screen;
static lv_obj_t* time_label;
static lv_obj_t* ip_label;
static lvc_calendar_t* calendar;
static lvc_events_panel_t* events_panel;
static lv_style_t test_style;

static char ip_addr[INET_ADDRSTRLEN];


void check_time_timer(lv_timer_t* timer);
void update_events_timer(lv_timer_t* timer);
void update_time_label(lv_event_t* event);
void get_ip(char* buf, size_t len);


void create_styles(void)
{
    lv_style_init(&test_style);
    lv_style_set_bg_color(&test_style, lv_color_hex(0x00ff66));
    lv_style_set_bg_opa(&test_style, LV_OPA_COVER);
}

void create_widgets(void)
{
    get_ip(ip_addr, INET_ADDRSTRLEN);
    printf("IP: %s\n", ip_addr);

    create_styles();


    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xffffff), LV_PART_MAIN);

	time_label = lv_label_create(screen);
	lv_label_set_text(time_label, "Time goes here");
	lv_obj_align(time_label, LV_ALIGN_TOP_LEFT, 5, 5);

    ip_label = lv_label_create(screen);
    lv_label_set_text_fmt(ip_label, "IP: %s", ip_addr);
	lv_obj_align(ip_label, LV_ALIGN_BOTTOM_LEFT, 5, -5);

    events_panel = create_events_panel(screen);
    lv_obj_set_size(events_panel->panel, disp_hpercent_to_px(20), disp_height);
    lv_obj_align(events_panel->panel, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_border_width(events_panel->panel, 2, LV_PART_MAIN);
    lv_obj_set_style_border_side(events_panel->panel, LV_BORDER_SIDE_LEFT, LV_PART_MAIN);
    lv_obj_set_style_radius(events_panel->panel, 0, LV_PART_MAIN);

    // Calendar must be created last. It calls the function to update the events
    uint32_t cal_width = disp_hpercent_to_px(70);
    calendar = create_calendar(screen, cal_width);
    lv_obj_align(calendar->calendar, LV_ALIGN_LEFT_MID, disp_hpercent_to_px(5), 0);


    lv_screen_load(screen);


	lv_obj_add_event_cb(time_label, update_time_label, LV_EVENT_REFRESH, NULL);

	lv_timer_create(check_time_timer, 100, NULL);
	// lv_timer_create(update_events_timer, 1 * 60 * 1000, NULL);
}

void destroy_widgets(void)
{
    destroy_calendar(calendar);
    destroy_events_panel(events_panel);

    lv_obj_clean(screen);
}


void check_time_timer(lv_timer_t* timer)
{
	(void) timer;

	static int8_t secs = -1;
    static int8_t min = -1;
    static int8_t day = -1;

	time_t t = time(NULL);
	struct tm* ct = localtime(&t);

    // Prevent the calendar from being redrawn on the first call of this timer
    if (day == -1) {
        day = ct->tm_mday;
        min = ct->tm_min;
    }

	if (secs != ct->tm_sec) {
		secs = ct->tm_sec;
		lv_obj_send_event(time_label, LV_EVENT_REFRESH, ct);
	}

    if (day != ct->tm_mday) {
        day = ct->tm_mday;
        min = ct->tm_min;
        lv_obj_send_event(calendar->calendar, LV_EVENT_REFRESH, ct);
    } else if (min != ct->tm_min) {
        min = ct->tm_min;
        updateEvents(ct->tm_mday, ct->tm_mon, ct->tm_year + 1900);

        get_ip(ip_addr, INET_ADDRSTRLEN);
        lv_label_set_text_fmt(ip_label, "IP: %s", ip_addr);
    }
}

void update_events_timer(lv_timer_t* timer)
{
    (void) timer;

    time_t t = time(NULL);
    struct tm* ct = localtime(&t);

    updateEvents(ct->tm_mday, ct->tm_mon, ct->tm_year + 1900);
}


void update_time_label(lv_event_t* event)
{
	struct tm* ct = lv_event_get_param(event);

	char time_str[TIME_BUF_LEN];
	strftime(time_str, TIME_BUF_LEN, "%I:%M:%S %p", ct);
	lv_label_set_text(time_label, time_str);
}

void get_ip(char* buf, size_t len)
{
    struct ifaddrs* ifAddrStruct = NULL;
    struct ifaddrs* ifa = NULL;
    bool addrFound = false;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }

        // I only want IPv4
        if (ifa->ifa_addr->sa_family == AF_INET) {
            void* tmpAddrPtr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, buf, len);
            
            // Skip loopback addresses
            if (buf[0] == '1' && buf[1] == '2' && buf[2] == '7') {
                continue;
            }

            addrFound = true;
            break;
        } 
    }

    if (!addrFound) {
        buf[0] = '\0';
    }

    if (ifAddrStruct != NULL) {
        freeifaddrs(ifAddrStruct);
    }
}

