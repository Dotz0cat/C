#include "rajio.h"

//prototypes
void set_message_handlers(GstBus *bus, const char* sql_file);
void error_handler(GstBus *bus, GstMessage *msg, gpointer data);
void eos_handler(GstBus *bus, GstMessage *msg, gpointer data);

//external prototypes
extern int get_highest_id(char* file_name);
extern int start_playing(int station_id);


void set_message_handlers(GstBus *bus, const char* sql_file) {
	gst_bus_add_signal_watch(bus);
	g_signal_connect(bus, "message::error", G_CALLBACK(error_handler), NULL);
	g_signal_connect(bus, "message::eos", G_CALLBACK(eos_handler), sql_file);
}

void error_handler(GstBus *bus, GstMessage *msg, gpointer data) {

}

void eos_handler(GstBus *bus, GstMessage *msg, gpointer data) {
	gst_element_set_state(pipeline, GST_STATE_READY);
	if (most_recent_id == get_highest_id(data)) {
		most_recent_id = 1;
	}
	else {
		most_recent_id++;
	}

	if (start_playing(most_recent_id) != 0) {
		fprintf(stderr, "there was a error");
	}

	return;
}
