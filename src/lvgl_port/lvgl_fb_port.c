#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include <pthread.h>
#include <sched.h>

#include <lvgl.h>

#include "fbd.h"
#include "ui.h"


uint32_t disp_width = 0;
uint32_t disp_height = 0;


static volatile sig_atomic_t running = 1;


static void sig_handler(int sig);
void* tick_thread(void* arg);


void fb_flush_cb(lv_display_t* display, const lv_area_t* area, uint8_t* px_map)
{
	struct framebuf* fb = (struct framebuf*)lv_display_get_user_data(display);

	int32_t area_width_b = (area->x2 - area->x1 + 1) * 2; // Two bytes per pixel

	
	for (int32_t y = area->y1; y <= area->y2; y++) {
		// memcpy is faster than copying manually
		memcpy(fb->buf + (y * fb->xres) + area->x1, px_map, area_width_b);
		px_map += area_width_b;
	}

	lv_display_flush_ready(display);
}

uint32_t disp_hpercent_to_px(uint32_t percent)
{
    return (disp_width * percent) / 100;
}

uint32_t disp_vpercent_to_px(uint32_t percent)
{
    return (disp_height * percent) / 100;
}

int lvgl_fb_run(const char* const fb_dev)
{
    struct sigaction act;
    act.sa_handler = sig_handler;
    sigaction(SIGINT, &act, NULL);


	struct framebuf fb;
	int ret = open_fb(&fb, fb_dev);
	if (ret != 0) {
		printf("Failed to open framebuf. Error: %d\n", ret);
		return ret;
	}

	clear_fb(&fb);

    disp_width = fb.xres;
    disp_height = fb.yres;

    printf("Opened framebuffer %s with dimensions %dx%d\n", fb_dev, disp_width, disp_height);

	lv_init();

	lv_display_t* display = lv_display_create(disp_width, disp_height);
	lv_display_set_flush_cb(display, fb_flush_cb);
	lv_display_set_user_data(display, &fb);

	uint16_t* buf1 = (uint16_t*)malloc(fb.size);
	lv_display_set_buffers(display, buf1, NULL, fb.size, LV_DISPLAY_RENDER_MODE_PARTIAL);


	create_widgets();

	// Copied from https://stackoverflow.com/a/27558789
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_getschedparam(&attr, &param);
	(param.sched_priority)++;
	pthread_attr_setschedparam(&attr, &param);

	running = true;

	pthread_t thread_id;
	pthread_create(&thread_id, &attr, tick_thread, NULL);


	while (running) {
		uint32_t time_till_next = lv_task_handler();
		usleep(time_till_next * 1000);
	}

    printf("\nStopping gracefully\n");


	pthread_join(thread_id, NULL);

    destroy_widgets();

	free(buf1);
	
	close_fb(&fb);

	return 0;
}

void* tick_thread(void* arg)
{
	(void) arg;

	while (running) {
		usleep(5 * 1000);
		lv_tick_inc(5);
	}

	return NULL;
}

static void sig_handler(int sig)
{
    (void) sig;

    running = 0;
}

