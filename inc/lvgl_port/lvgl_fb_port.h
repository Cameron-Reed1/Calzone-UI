#pragma once

#include <stdint.h>

#include <lvgl.h>


extern uint32_t disp_height;
extern uint32_t disp_width;


void fb_flush_cb(lv_display_t* display, const lv_area_t* area, uint8_t* px_map);
uint32_t disp_hpercent_to_px(uint32_t percent);
uint32_t disp_vpercent_to_px(uint32_t percent);
int lvgl_fb_run(const char* const fb_dev);

