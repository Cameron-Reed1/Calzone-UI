#include <argparser.h>

#include "lvgl_port/lvgl_fb_port.h"


int main(int argc, char** argv)
{
    using namespace Cam::Arguments;
    
    Parser argParser("calzone_ui");
    argParser.set_description("LVGL frontend for Calzone");

    Option fbdev("fb", "Framebuffer device e.g. /dev/fb0", STRING);

    argParser.add_option(&fbdev);

    ERROR argsErr = argParser.parse(argc, argv);
    if (argsErr == SPECIAL_CASE_HELP) {
        return 0;
    }

    if (argsErr != NO_ERROR) {
        return -1;
    }


    if (fbdev.found()) {
        return lvgl_fb_run((const char*)fbdev.data);
    } else {
	    return lvgl_fb_run("/dev/fb0");
    }
}

