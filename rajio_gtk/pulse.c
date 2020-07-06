#include "rajio.h"

int pulseaudio_init(void);
int pulseaudio_deinit(void);

//callbacks
void context_state_cb(pa_context* context, void* mainloop);
void stream_state_cb(pa_stream* stream, void* mainloop);
void stream_success_cb(pa_stream* stream, int success, void* userdata);
void stream_write_cb(pa_stream* stream, size_t requested_bytes, void* mainloop);

//delcrations
pa_threaded_mainloop* mainloop;
pa_context* context_pa;


int pulseaudio_init() {

	pa_mainloop_api* mainloop_api;

	mainloop = pa_threaded_mainloop_new();
    if (!mainloop) {
        fprintf(stderr, "cant make mainloop\r\n");
        return -1;
    }

    mainloop_api = pa_threaded_mainloop_get_api(mainloop);
    if (!mainloop_api) {
        fprintf(stderr, "cant get mainloop api\r\n");
        return -1;
    }

    context_pa = pa_context_new(mainloop_api, "internet radio");

    pa_context_set_state_callback(context_pa, context_state_cb, mainloop);

    pa_threaded_mainloop_lock(mainloop);

    if (pa_threaded_mainloop_start(mainloop) != 0) {
        fprintf(stderr, "cant start mainloop\r\n");
        return -1;
    }

    if (pa_context_connect(context_pa, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) != 0) {
        fprintf(stderr, "cant connect pa context\r\n");
        return -1;
    }

    //check if the context state is good
	for (;;) {
        pa_context_state_t context_state = pa_context_get_state(context_pa);
        if (PA_CONTEXT_IS_GOOD(context_state) == 0) {
            fprintf(stderr, "context state is broke\r\n");
            return NULL;
        }
        //printf("context state: %i\r\n", context_state);
        if (context_state == PA_CONTEXT_READY) break;
        pa_threaded_mainloop_wait(mainloop);
    }

    return 0;
}

int pulseaudio_deinit() {
	pa_context_disconnect(context_pa);
    pa_context_unref(context_pa);
    pa_threaded_mainloop_free(mainloop);

    return 0;
}

void context_state_cb(pa_context* context, void* mainloop_a) {
    pa_threaded_mainloop_signal((pa_threaded_mainloop*)mainloop_a, 0);
}

void stream_state_cb(pa_stream* stream, void* mainloop_a) {
    pa_threaded_mainloop_signal((pa_threaded_mainloop*)mainloop_a, 0);
}

void stream_success_cb(pa_stream* stream, int success, void* userdata) {
    return;
}

void stream_write_cb(pa_stream* stream, size_t requested_bytes, void* mainloop_a) {

    if (requested_bytes >= 8 * 1152) {
        pa_threaded_mainloop_signal((pa_threaded_mainloop*)mainloop_a, 0);
    }

    return;
}
