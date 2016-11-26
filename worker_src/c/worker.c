#include <pebble_worker.h>
// Used to identify the source of a message
#define SOURCE_FOREGROUND 0
#define SOURCE_BACKGROUND 1

static int s_some_value = 3;
static int s_another_value = 4;


static void worker_message_handler(uint16_t type, 
                                    AppWorkerMessage *message) {
  if(type == SOURCE_FOREGROUND) {
    // Get the data, if it was sent from the foreground
    s_some_value = message->data0;
    s_another_value = message->data1;
  }
}


static void prv_init() {
  // Initialize the worker here
  // Construct a message to send
AppWorkerMessage message = {
  .data0 = s_some_value,
  .data1 = s_another_value
};
  
  // Subscribe to get AppWorkerMessages
app_worker_message_subscribe(worker_message_handler);

// Send the data to the foreground app
app_worker_send_message(SOURCE_BACKGROUND, &message);

}

static void prv_deinit() {
  // Deinitialize the worker here
  // Stop the background worker

}

int main(void) {
  prv_init();
  worker_event_loop();
  prv_deinit();
}

