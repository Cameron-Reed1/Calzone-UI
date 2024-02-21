#include "lvgl_port/lvgl_fb_port.h"
#include <stdio.h>


int main()
{
	return lvgl_fb_run("/dev/fb0");
}

