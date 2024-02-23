#pragma once

#include <stdint.h>


struct framebuf {
	uint32_t xres;
	uint32_t yres;
	uint32_t size;

	int fd;
	uint16_t* buf;
};

int open_fb(framebuf* fb, const char* const dev);
void close_fb(framebuf* fb);
void clear_fb(framebuf* fb);

