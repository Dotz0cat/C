#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <pulse/pulseaudio.h>
#include <pthread.h>

//delcrations for pulse stuff
extern pa_threaded_mainloop* mainloop;
extern pa_context* context_pa;

extern int most_recent_id;
extern int keepalive;
extern pthread_t thread_id;
