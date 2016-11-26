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
static int steps = -1;
static char step_string[12] = "";
static bool inProgress = false;
static bool s_js_ready;

bool comm_is_js_ready() {
  return s_js_ready;
}

static void display_steps() {
  // Create an array to store data
  
  //steps = (int)health_service_sum_today(metric);
  if(inProgress) {
    snprintf(step_string, 12, "%d", steps++);
    text_layer_set_text(s_time_layer, step_string);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  display_steps();
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  // A message was received, but had to be dropped
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped. Reason: %d", (int)reason);
}

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
    Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY_JSReady);
  APP_LOG(APP_LOG_LEVEL_INFO, "Inbox Received");
  if(ready_tuple) {
    // PebbleKit JS is ready! Safe to send messages
    s_js_ready = true;
    
    APP_LOG(APP_LOG_LEVEL_INFO, "JSReady Received");
  }
  
  Tuple *start_tuple = dict_find(iter, MESSAGE_KEY_Start);
  if(start_tuple) {
    int32_t start_no = start_tuple->value->int32;
    
    APP_LOG(APP_LOG_LEVEL_INFO, "Start No Received: %d", (int)start_no);
    
    steps = (int) start_no;
  }
}

static void outbox_sent_callback(DictionaryIterator *iter, void *context) {
  // The message just sent has been successfully delivered

}

static void outbox_failed_callback(DictionaryIterator *iter,
                                      AppMessageResult reason, void *context) {
  // The message just sent failed to be delivered
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message send failed. Reason: %d", (int)reason);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // A single click has just occured
  inProgress = !inProgress;
  
  if(!inProgress) {
    //Send current message to phone
    // Declare the dictionary's iterator
    DictionaryIterator *out_iter;
    
    // Prepare the outbox buffer for this message
    AppMessageResult result = app_message_outbox_begin(&out_iter);
    if(result == APP_MSG_OK) {
      dict_write_int(out_iter, MESSAGE_KEY_CurrentCount, &steps, sizeof(int), true);
    
      // Send this message
      result = app_message_outbox_send();
      if(result != APP_MSG_OK) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
      }
    } else {
      // The outbox cannot be used right now
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
    }
  }
}

static void click_config_provider(void *context) {
  ButtonId id = BUTTON_ID_SELECT;  // The Select button

  window_single_click_subscribe(id, select_click_handler);
}

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
  APP_LOG(APP_LOG_LEVEL_INFO, "Initializing");
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
  
  text_layer_set_text(s_time_layer, "Start");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  /*
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
*/  
  window_set_click_config_provider(s_window, click_config_provider);
  
  
  // Register to be notified about inbox received events
app_message_register_inbox_received(inbox_received_callback);
// Register to be notified about inbox dropped events
app_message_register_inbox_dropped(inbox_dropped_callback);
 
  // Register to be notified about outbox sent events
app_message_register_outbox_sent(outbox_sent_callback);
  
  // Register to be notified about outbox failed events
app_message_register_outbox_failed(outbox_failed_callback);

  // Open AppMessage
const int inbox_size = 128;
const int outbox_size = 128;
app_message_open(inbox_size, outbox_size);

  
  // Show the Window on the watch, with animated=true
  window_stack_push(s_window, true);
  
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  
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