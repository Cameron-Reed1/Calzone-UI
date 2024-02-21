#include <lvgl.h>


typedef struct {
    lv_obj_t* panel;
    lv_obj_t* header;
    lv_obj_t** event_labels;
    uint16_t event_labels_count;
} lvc_events_panel_t;


lvc_events_panel_t* create_events_panel(lv_obj_t* parent);
void destroy_events_panel(lvc_events_panel_t* panel);
