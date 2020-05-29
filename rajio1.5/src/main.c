#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fnmatch.h>
#include <arpa/inet.h>
//#include <AL/al.h>
//#include <AL/alc.h>
#include <pulse/simple.h>
#include <pulse/pulseaudio.h>
#include <signal.h>
#include <string.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libswresample/swresample.h>


void print_help(void);
int play_with_libav(char *url);

//callbacks for async pulseaduio
void context_state_cb(pa_context* context, void* mainloop);
void stream_state_cb(pa_stream* stream, void* mainloop);
void stream_success_cb(pa_stream* stream, int success, void* userdata);
void stream_write_cb(pa_stream* stream, size_t requested_bytes, void* userdata);
void stream_underflow_cb(pa_stream* stream, void* mainloop);
void stream_overflow_cb(pa_stream* stream, void* mainloop);

/*
    my useal test case is http://158.69.38.195:20278
    http://184.75.223.178:8102
*/
int main(int argc, char *argv[])
{
    int success;
    // print help if -h
    if (argc==0) {
       print_help();
       return 0;
    }
    else if (argc!=0) {
        switch (argc) {
        case (2):
            #ifdef DEBUG
                printf(argv[1]);
            #endif
            success = play_with_libav(argv[1]);
            return success;
        default:
            print_help();
            return 0;
        }
    }
    success = 0;

    return success;
}

void print_help() {
    printf("the proper way to use this program at the moment is with the syantax\r\n");
    printf("rajio protocol://url:port\r\n");
}

int play_with_libav(char *url) {
    //asysnc stuff
    pa_threaded_mainloop* mainloop;
    pa_mainloop_api* mainloop_api;
    pa_context* context_pa;
    pa_stream* stream_pa;

    uint8_t* audio_buffer;
    int samples = 1152;
    int holder;
    char err[50];
    int error;
    error = 0;

    audio_buffer = malloc((11520) * sizeof(uint8_t));
    if (!audio_buffer) {
        fprintf(stderr, "cant get memory for audio buffer\r\n");
        return -1;
    }

    //make a mainloop for pulse
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

    pa_context_set_state_callback(context_pa, &context_state_cb, mainloop);

    pa_threaded_mainloop_lock(mainloop);

    if (pa_threaded_mainloop_start(mainloop) != 0) {
        fprintf(stderr, "cant start mainloop\r\n");
        return -1;
    }

    if (pa_context_connect(context_pa, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) != 0) {
        fprintf(stderr, "cant connect pa context\r\n");
        return -1;
    }

    for (;;) {
        pa_context_state_t context_state = pa_context_get_state(context_pa);
        if (PA_CONTEXT_IS_GOOD(context_state) == 0) {
            fprintf(stderr, "context state is broke\r\n");
            return -1;
        }
        printf("context state: %i\r\n", context_state);
        if (context_state == PA_CONTEXT_READY) break;
        pa_threaded_mainloop_wait(mainloop);
    }

    //make the playback stream
    pa_sample_spec sample_spec;
    sample_spec.format = PA_SAMPLE_FLOAT32LE;
    sample_spec.rate = 44100;
    sample_spec.channels = 2;

    pa_channel_map map;
    pa_channel_map_init_stereo(&map);

    stream_pa = pa_stream_new(context_pa, "rajio", &sample_spec, &map);
    pa_stream_set_state_callback(stream_pa, &stream_state_cb, mainloop);
    pa_stream_set_write_callback(stream_pa, &stream_write_cb, mainloop);
    pa_stream_set_underflow_callback(stream_pa, &stream_underflow_cb, mainloop);
    pa_stream_set_overflow_callback(stream_pa, &stream_overflow_cb, mainloop);

    pa_buffer_attr buffer_attr;
    buffer_attr.maxlength = (uint32_t) -1; //pa_usec_to_bytes(20000, &sample_spec) * 10;
    buffer_attr.tlength = (uint32_t) pa_usec_to_bytes(10000, &sample_spec);
    printf("%zu\r\n", pa_usec_to_bytes(10000, &sample_spec));
    buffer_attr.prebuf = (uint32_t) 0;
    buffer_attr.minreq = (uint32_t) pa_usec_to_bytes(0, &sample_spec);

    //copied from https://stackoverflow.com/questions/29977651/how-can-the-pulseaudio-asynchronous-library-be-used-to-play-raw-pcm-data
    pa_stream_flags_t stream_flags;
    stream_flags = PA_STREAM_START_CORKED | PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_AUTO_TIMING_UPDATE | PA_STREAM_ADJUST_LATENCY;

    if (pa_stream_connect_playback(stream_pa, NULL, &buffer_attr, stream_flags, NULL, NULL) != 0) {
        fprintf(stderr, "could not connect to playback stream\r\n");
        return -1;
    }

    //libav stuff
    avformat_network_init();


    AVFormatContext* format = avformat_alloc_context();


    if ((holder = avformat_open_input(&format, url, NULL, NULL))!=0) {
        fprintf(stderr, "avformat_open_input broke\r\n");
        return holder;
    }
    if ((holder = avformat_find_stream_info(format, NULL))<0) {
        fprintf(stderr, "find_stream_info broke\r\n");
        fprintf(stderr, "%i\r\n", holder);
        av_make_error_string(err, sizeof(err), holder);
        fprintf(stderr, "%s\r\n", err);
        return holder;
    }

    size_t stream = 0;

    for (; stream < format->nb_streams; stream++) {
        if (format->streams[stream]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            break;
        }
    }
    if (stream == format->nb_streams) {
        fprintf(stderr, "cant find stream or something\r\n");
        return -1;
    }

    AVCodecParameters* codec_prams = format->streams[stream]->codecpar;
    if (!codec_prams) {
        fprintf(stderr, "codec params not working\r\n");
        return -1;
    }


    AVCodecContext* codec_context = avcodec_alloc_context3(NULL);

    holder = avcodec_parameters_to_context(codec_context, codec_prams);
    if (holder != 0) {
        fprintf(stderr, "something is wrong\r\n");
        return holder;
    }
    if (codec_context->channel_layout == 0) {
        codec_context->channel_layout = AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT;
    }

    AVCodec* codec1 = avcodec_find_decoder(codec_prams->codec_id);
    if (!codec1) {
        fprintf(stderr, "codec is broke\r\n");
        return -1;
    }

    holder = avcodec_open2(codec_context, codec1, NULL);
    if (holder<0) {
        fprintf(stderr, "error avopen context\r\n");
        return holder;
    }



    SwrContext* swr_context = swr_alloc_set_opts(NULL, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT, AV_SAMPLE_FMT_FLT, (int)sample_spec.rate, (long)codec_context->channel_layout, codec_context->sample_fmt, codec_context->sample_rate, 0, NULL);
    if (!swr_context) {
        fprintf(stderr, "swr context broke\r\n");
        return -1;
    }
    swr_init(swr_context);

    AVPacket avpkt;
    av_init_packet(&avpkt);
    avpkt.data = NULL;
    avpkt.size = 0;

    AVFrame* frame = av_frame_alloc();
    //AVFrame* frame_out = av_frame_alloc();

    holder = 0;

    for (;;) {
        pa_stream_state_t stream_state = pa_stream_get_state(stream_pa);
        if (PA_CONTEXT_IS_GOOD(stream_state) == 0) {
            fprintf(stderr, "stream state is broke\r\n");
            return -1;
        }
        printf("stream state: %i\r\n", stream_state);
        if (stream_state == PA_STREAM_READY) break;
        pa_threaded_mainloop_wait(mainloop);
    }

    //pa_threaded_mainloop_unlock(mainloop);

    error = 0;

    pa_usec_t user_spec; // = pa_bytes_to_usec((uint64_t) 11520, &sample_spec);


    printf("latancy: %i\r\n", pa_stream_get_latency(stream_pa, &user_spec, &error));
    printf("%lu\r\n", user_spec);
    printf("error: %i\r\n", error);
    printf("%zu\r\n", pa_bytes_per_second(&sample_spec));

    pa_stream_cork(stream_pa, 0, &stream_success_cb, mainloop);

    /*frame_out->channel_layout = AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT;
    frame_out->format = AV_SAMPLE_FMT_FLT;
    frame_out->sample_rate = (int)sample_spec.rate;*/

    //int64_t runs = 1;

    while (av_read_frame(format, &avpkt) >= 0) {
        if (avpkt.stream_index != (int)stream) {
            continue;
        }

        if (avcodec_send_packet(codec_context, &avpkt) < 0) {
            fprintf(stderr, "cant send packet\r\n");
            return -1;
        }

        if (avcodec_receive_frame(codec_context, frame) < 0) {
            fprintf(stderr, "receive frame\r\n");
            return -1;
        }

        holder = swr_convert(swr_context, &audio_buffer, samples, (const uint8_t**)&frame->data, frame->nb_samples);
        if (holder < 0) {
            fprintf(stderr, "swrconvert not working\r\n");
            return -1;
        }

        //printf("%i\r\n", frame->nb_samples);

        //pa_stream_trigger(stream_pa, stream_success_cb, mainloop);

         while (holder>0) {

            //if (pa_stream_is_corked(stream_pa)==0) {
                //pa_threaded_mainloop_wait(mainloop);
            //}

            pa_threaded_mainloop_wait(mainloop);

            pa_threaded_mainloop_unlock(mainloop);

            if ((error = pa_stream_write(stream_pa, audio_buffer, (11520), NULL, 0, PA_SEEK_RELATIVE)) != 0) {
                fprintf(stderr, "error writing to pa stream\r\n");
                fprintf(stderr, "%s\r\n", pa_strerror(error));
                fprintf(stderr, "%s\r\n", pa_strerror(pa_context_errno(context_pa)));
                return -1;
            }

            pa_threaded_mainloop_lock(mainloop);

            //pa_stream_drain(stream_pa, NULL, mainloop);

            holder = swr_convert(swr_context, &audio_buffer, samples, NULL, 0);
            if (holder < 0) {
                fprintf(stderr, "swr convert on line 174");
                return -1;
            }
         }



         /*if (swr_convert_frame(swr_context, frame_out, frame) < 0) {
            fprintf(stderr, "swr broke\r\n");
            return -1;
         }

         pa_threaded_mainloop_wait(mainloop);

         pa_threaded_mainloop_unlock(mainloop);

         if ((error = pa_stream_write(stream_pa, frame_out->data, sizeof(frame_out->data), NULL, 0, PA_SEEK_RELATIVE)) != 0) {
                fprintf(stderr, "error writing to pa stream\r\n");
                fprintf(stderr, "%s\r\n", pa_strerror(error));
                fprintf(stderr, "%s\r\n", pa_strerror(pa_context_errno(context_pa)));
                return -1;
        }

        pa_threaded_mainloop_lock(mainloop);*/

        /*if (pa_stream_is_corked(stream_pa)==1) {
            runs += 1;
        }

        //printf("%li\r\n", runs);

        if ((runs%62)==0) {
            if (pa_stream_is_corked(stream_pa)==1) {
                pa_stream_cork(stream_pa, 0, &stream_success_cb, mainloop);
            }

        }*/

        av_packet_unref(&avpkt);
    }



    printf("going well add more\r\n");

    free(audio_buffer);
    return 0;
}

void context_state_cb(pa_context* context, void* mainloop) {

    /*pa_context_state_t state;
    int* pa_ready = userdata;
    state = pa_context_get_state(context);
    switch (state) {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
        default:
            break;
        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
            *pa_ready = 2;
            break;
        case PA_CONTEXT_READY:
            *pa_ready = 1;
            break;
    }*/

    //dont know what this does so will comment it out
    pa_threaded_mainloop_signal((pa_threaded_mainloop*)mainloop, 0);
}

void stream_state_cb(pa_stream* stream, void* mainloop) {
    pa_threaded_mainloop_signal((pa_threaded_mainloop*)mainloop, 0);
}

void stream_success_cb(pa_stream* stream, int success, void* userdata) {
    return;
}

void stream_write_cb(pa_stream* stream, size_t requested_bytes, void* mainloop) {

    //printf("i am called the callback\r\n");

    pa_threaded_mainloop_signal((pa_threaded_mainloop*)mainloop, 0);
}

void stream_underflow_cb(pa_stream* stream, void* mainloop) {
    printf("underflow\r\n");
    //pa_threaded_mainloop_lock(mainloop);
    //pa_stream_cork(stream, 1, &stream_success_cb, mainloop);
    //pa_threaded_mainloop_unlock(mainloop);
}

void stream_overflow_cb(pa_stream* stream, void* mainloop) {
    printf("overflow\r\n");
    //pa_stream_trigger(stream, &stream_success_cb, mainloop);
}

