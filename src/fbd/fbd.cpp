#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>

#include "fbd.h"

int open_fb(framebuf* fb, const char* const dev)
{
	fb->fd = open(dev, O_RDWR);
	if (fb->fd < 0) {
		printf("Failed to open framebuffer device\n");
		return -1;
	}

	// fb_fix_screeninfo fix_info;
	// ioctl(fd, FBIOGET_FSCREENINFO, &fix_info);

	fb_var_screeninfo var_info;
	ioctl(fb->fd, FBIOGET_VSCREENINFO, &var_info);

	fb->xres = var_info.xres;
	fb->yres = var_info.yres;
	fb->size = fb->xres * fb->yres * (var_info.bits_per_pixel / 8);

	fb->buf = (uint16_t*) mmap(0, fb->size, PROT_READ | PROT_WRITE, MAP_SHARED, fb->fd, 0);
	if (fb->buf == MAP_FAILED) {
		printf("Failed to map framebuf\n");
		return -2;
	}

	return 0;
}

void close_fb(framebuf* fb)
{
	// memset(fb->buf, 0, fb->size);
	munmap(fb->buf, fb->size);
	close(fb->fd);

	fb->xres = 0;
	fb->yres = 0;
	fb->size = 0;
	fb->buf = 0;
	fb->fd = -1;
}

void clear_fb(framebuf* fb)
{
	memset(fb->buf, 0, fb->size);
}

