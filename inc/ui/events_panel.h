#include <lvgl.h>
#include <vector>


class EventsPanel {
public:
    EventsPanel(lv_obj_t* parent);
    ~EventsPanel();
    void Style();

public:
    lv_obj_t* panel;

private:
    static void DrawEventList_cb(lv_event_t* event);

private:
    lv_obj_t* header;
    std::vector<lv_obj_t*> event_labels;
};

