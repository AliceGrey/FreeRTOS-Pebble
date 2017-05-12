/* systemapp.c
 * routines for [...]
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "systemapp.h"


const char *systemapp_name = "System";

static void systemapp_update_proc(Layer *layer, GContext *ctx);
void systemapp_main(void);

static Window *s_main_window;
static Layer *s_canvas_layer;

void systemapp_config_provider(Window *window);
void up_single_click_handler(ClickRecognizerRef recognizer, void *context);
void down_single_click_handler(ClickRecognizerRef recognizer, void *context);
void select_single_click_handler(ClickRecognizerRef recognizer, void *context);
void back_click_handler(ClickRecognizerRef recognizer, void *context);

typedef struct {
    uint8_t hours;
    uint8_t minutes;
} Time;

static Time s_last_time;

GBitmap* gbitmap = NULL;

static void systemapp_window_load(Window *window)
{
    printf("WF load\n");
    Layer *window_layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);

    s_canvas_layer = layer_create(bounds);
    layer_set_update_proc(s_canvas_layer, systemapp_update_proc);
    layer_add_child(window_layer, s_canvas_layer);
   
    
    //layer_mark_dirty(s_canvas_layer);
    
    //tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
}


static void systemapp_window_unload(Window *window)
{
    layer_destroy(s_canvas_layer);
}

void systemapp_init(void)
{
    printf("init\n");
    s_main_window = window_create();
    menu_init();
    
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = systemapp_window_load,
        .unload = systemapp_window_unload,
    });
    
    window_stack_push(s_main_window, true);
    
    window_set_click_config_provider(s_main_window, (ClickConfigProvider)systemapp_config_provider);
    menu_show(0, 0);
}

void systemapp_deinit(void)
{
    window_destroy(s_main_window);
}


void systemapp_main(void)
{
    systemapp_init();
    app_event_loop();
    systemapp_deinit();
}

void systemapp_tick(void)
{
    struct tm *tick_time = rbl_get_tm();
    
    printf("system\n");
    // Store time
    s_last_time.hours = tick_time->tm_hour;
    s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
    s_last_time.minutes = tick_time->tm_min;

    // Redraw
    if (s_canvas_layer)
    {
//         layer_mark_dirty(s_canvas_layer);
    }
}

void systemapp_config_provider(Window *window)
{
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 600, down_single_click_handler);
    window_single_repeating_click_subscribe(BUTTON_ID_UP, 600, up_single_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
    window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}

void down_single_click_handler(ClickRecognizerRef recognizer, void *context)
{
    menu_down();
    layer_mark_dirty(s_canvas_layer);
}

void up_single_click_handler(ClickRecognizerRef recognizer, void *context)
{
    menu_up();
    layer_mark_dirty(s_canvas_layer);
}

void select_single_click_handler(ClickRecognizerRef recognizer, void *context)
{
    menu_select();
    layer_mark_dirty(s_canvas_layer);
}

void back_click_handler(ClickRecognizerRef recognizer, void *context)
{
    menu_back();
    layer_mark_dirty(s_canvas_layer);
}

static void systemapp_update_proc(Layer *layer, GContext *nGContext)
{   
    menu_show(0, 0);    
}


