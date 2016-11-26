#include <pebble.h>
#include <stdlib.h>
#include <stdio.h>

// Used to identify the source of a message
#define SOURCE_FOREGROUND 0
#define SOURCE_BACKGROUND 1

static int s_some_value = 1;
static int s_another_value = 2;
static Window *s_window;
static TextLayer *s_time_layer;
static int steps = 0;

static void main_window_load(Window *window) {
    // Get information about the Window

}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
}

static void worker_message_handler(uint16_t type, 
                                    AppWorkerMessage *message) {
  if(type == SOURCE_BACKGROUND) {
    // Get the data, only if it was sent from the background
    s_some_value = message->data0;
    s_another_value = message->data1;
  }
}

static void init() {
    // Create main Window element and assign to pointer
  s_window = window_create();
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  
  //char step_string[12] = "";
  
  //snprintf(step_string, 9, "Steps: %d", steps);
  
  text_layer_set_text(s_time_layer, "Test");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  
  // Launch the background worker
AppWorkerResult result = app_worker_launch();
  // Subscribe to get AppWorkerMessages
app_worker_message_subscribe(worker_message_handler);

// Construct a message to send
AppWorkerMessage message = {
  .data0 = s_some_value,
  .data1 = s_another_value
};

// Send the data to the background app
app_worker_send_message(SOURCE_FOREGROUND, &message);

  // Show the Window on the watch, with animated=true
  window_stack_push(s_window, true);
}

static void deinit() {
  // Stop the background worker
AppWorkerResult result = app_worker_kill();
  
    // Destroy Window
  window_destroy(s_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}