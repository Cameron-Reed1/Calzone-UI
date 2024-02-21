#pragma once

#include <stdint.h>


struct framebuf {
	uint32_t xres;
	uint32_t yres;
	uint32_t size;

	int fd;
	uint16_t* buf;
};

int open_fb(struct framebuf* fb, const char* const dev);
void close_fb(struct framebuf* fb);
void clear_fb(struct framebuf* fb);
void set_pixel(struct framebuf* fb, uint32_t x, uint32_t y, uint16_t color);

