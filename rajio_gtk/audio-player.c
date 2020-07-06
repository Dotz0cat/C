#include "rajio.h"
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libswresample/swresample.h>
#include <unistd.h>

//prototypes
void* play(void* URL);

//calbacks
extern void stream_state_cb(pa_stream* stream, void* mainloop);
extern void stream_success_cb(pa_stream* stream, int success, void* userdata);
extern void stream_write_cb(pa_stream* stream, size_t requested_bytes, void* mainloop);

void* play(void* URL) {
	pa_stream* stream_pa;
	int holder;
	char err[256];

    //make the playback stream
    pa_sample_spec sample_spec;
    sample_spec.format = PA_SAMPLE_FLOAT32LE;
    sample_spec.rate = 44100;
    sample_spec.channels = 2;

    pa_channel_map map;
    pa_channel_map_init_stereo(&map);

    stream_pa = pa_stream_new(context_pa, "rajio", &sample_spec, &map);
    pa_stream_set_state_callback(stream_pa, stream_state_cb, mainloop);
    pa_stream_set_write_callback(stream_pa, stream_write_cb, mainloop);
    //pa_stream_set_underflow_callback(stream_pa, stream_underflow_cb, mainloop);
    //pa_stream_set_overflow_callback(stream_pa, stream_overflow_cb, mainloop);

    //let pulse decide on the sizes and stuff
    pa_buffer_attr buffer_attr;
    buffer_attr.maxlength = (uint32_t) -1;
    buffer_attr.tlength = (uint32_t) -1;
    buffer_attr.prebuf = (uint32_t) -1;
    buffer_attr.minreq = (uint32_t) -1;

    pa_stream_flags_t stream_flags;
    stream_flags = PA_STREAM_START_CORKED | PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_AUTO_TIMING_UPDATE | PA_STREAM_ADJUST_LATENCY;

    //connect the stream
    if (pa_stream_connect_playback(stream_pa, NULL, &buffer_attr, stream_flags, NULL, NULL) != 0) {
        fprintf(stderr, "%s\r\n", pa_strerror(pa_context_errno(context_pa)));
        return NULL;
    }

    //ffmpeg stuff
    avformat_network_init();

    //make a new format context
    AVFormatContext* format = avformat_alloc_context();

    //open the url with ffmpeg
    if ((holder = avformat_open_input(&format, (char*) URL, NULL, NULL)) != 0) {
        av_make_error_string(err, sizeof(err), holder);
        fprintf(stderr, "avformat_open_input: %s\r\n", err);
        return NULL;
    }
    //find info about the stream from the url
    if ((holder = avformat_find_stream_info(format, NULL)) < 0) {
        av_make_error_string(err, sizeof(err), holder);
        fprintf(stderr, "avformat_find_stream_info: %s\r\n", err);
        return NULL;
    }

    size_t stream = 0;

    //find the audio stream
    for (; stream < format->nb_streams; stream++) {
        if (format->streams[stream]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            break;
        }
    }
    if (stream == format->nb_streams) {
        fprintf(stderr, "cant find stream or something\r\n");
        return NULL;
    }

    //get the av codec params to make a new avcodec context
    AVCodecParameters* codec_prams = format->streams[stream]->codecpar;
    if (!codec_prams) {
        fprintf(stderr, "codec params not working\r\n");
        return NULL;
    }

    //fill with a empty codec context so it does not segfault
    AVCodecContext* codec_context = avcodec_alloc_context3(NULL);

    holder = avcodec_parameters_to_context(codec_context, codec_prams);
    if (holder != 0) {
        av_make_error_string(err, sizeof(err), holder);
        fprintf(stderr, "avcodec_parameters_to_context: %s\r\n", err);
        return NULL;
    }
    if (codec_context->channel_layout == 0) {
        codec_context->channel_layout = AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT;
    }

    AVCodec* codec1 = avcodec_find_decoder(codec_prams->codec_id);
    if (!codec1) {
        fprintf(stderr, "codec is broke\r\n");
        return NULL;
    }

    holder = avcodec_open2(codec_context, codec1, NULL);
    if (holder<0) {
        av_make_error_string(err, sizeof(err), holder);
        fprintf(stderr, "avcodec_open2: %s\r\n", err);
        return NULL;
    }


    //make a new swr context so i can use swr later
    SwrContext* swr_context = swr_alloc_set_opts(NULL, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT, AV_SAMPLE_FMT_FLT, (int)sample_spec.rate, (long)codec_context->channel_layout, codec_context->sample_fmt, codec_context->sample_rate, 0, NULL);
    if (!swr_context) {
        fprintf(stderr, "swr context broke\r\n");
        return NULL;
    }
    swr_init(swr_context);

    //make a avpacket to hold things test later
    AVPacket avpkt;
    av_init_packet(&avpkt);
    avpkt.data = NULL;
    avpkt.size = 0;

    AVFrame* frame = av_frame_alloc();

    holder = 0;

    for (;;) {
        pa_stream_state_t stream_state = pa_stream_get_state(stream_pa);
        if (PA_STREAM_IS_GOOD(stream_state) == 0) {
            fprintf(stderr, "stream state is broke\r\n");
            return NULL;
        }
        if (stream_state == PA_STREAM_READY) break;
        pa_threaded_mainloop_wait(mainloop);
    }

    int error = 0;

    pa_stream_cork(stream_pa, 0, stream_success_cb, mainloop);

    size_t size_thing = 8 * 1152;

    uint8_t* audio_buffer;

    pa_threaded_mainloop_unlock(mainloop);

    while (av_read_frame(format, &avpkt) >= 0 && keepalive == 1) {
        if (avpkt.stream_index != (int)stream) {
            continue;
        }

        if (avcodec_send_packet(codec_context, &avpkt) < 0) {
            fprintf(stderr, "cant send packet\r\n");
            return NULL;
        }

        if (avcodec_receive_frame(codec_context, frame) < 0) {
            fprintf(stderr, "receive frame\r\n");
            return NULL;
        }

        size_thing = 8 * 1152;

        int go = 0;

        while (go == 0) {

	        usleep(500);

	        pa_threaded_mainloop_wait(mainloop);

	        if (pa_stream_writable_size(stream_pa) >= 8 * 1152) {

	            if (pa_stream_begin_write(stream_pa, (void**) &audio_buffer, &size_thing) < 0) {
	                fprintf(stderr, "pa_begin_write failed: %s\r\n", pa_strerror(pa_context_errno(context_pa)));
	                return NULL;
	            }

	            holder = swr_convert(swr_context, &audio_buffer, 1152, (const uint8_t**)&frame->data, frame->nb_samples);
		        if (holder < 0) {
		            av_make_error_string(err, sizeof(err), holder);
		        	fprintf(stderr, "swr_convert: %s\r\n", err);
		            return NULL;
		        }

	            if ((error = pa_stream_write(stream_pa, audio_buffer, size_thing, NULL, 0, PA_SEEK_RELATIVE)) != 0) {
	                fprintf(stderr, "%s\r\n", pa_strerror(pa_context_errno(context_pa)));
	                return NULL;
	            }

	        }

	        go = 1;
    	}


        /*holder = swr_convert(swr_context, &audio_buffer, 1152, (const uint8_t**)&frame->data, frame->nb_samples);
        if (holder < 0) {
            av_make_error_string(err, sizeof(err), holder);
        	fprintf(stderr, "swr_convert: %s\r\n", err);
            return NULL;
        }

         while (holder>0) {

            usleep(500);

            pa_threaded_mainloop_wait(mainloop);

            //waits++;

            if (pa_stream_writable_size(stream_pa) >= buffer_size_macro * 1152) {

                if (pa_stream_begin_write(stream_pa, (void**) &audio_buffer, &size_thing) < 0) {
                    fprintf(stderr, "pa_begin_write failed: %s\r\n", pa_strerror(pa_context_errno(context_pa)));
                    return -1;
                }

                if ((error = pa_stream_write(stream_pa, audio_buffer, (buffer_size_macro*1152), NULL, 0, PA_SEEK_RELATIVE)) != 0) {
                    fprintf(stderr, "error writing to pa stream\r\n");
                    fprintf(stderr, "%s\r\n", pa_strerror(error));
                    fprintf(stderr, "%s\r\n", pa_strerror(pa_context_errno(context_pa)));
                    return -1;
                }

                //writes++;

                holder = swr_convert(swr_context, &audio_buffer, samples, NULL, 0);
                if (holder < 0) {
                    fprintf(stderr, "swr convert on line 174");
                    return -1;
                }

            }

         }*/

         /*printf("waits: %li\r\n", waits);
         printf("writes: %li\r\n", writes);

         printf("hit/miss ratio: %li\r\n", waits / writes);*/


        av_packet_unref(&avpkt);
    }

    //lock the pulse loop
    pa_threaded_mainloop_lock(mainloop);

    //clean up ffmpeg
    av_frame_free(&frame);
    swr_close(swr_context);
    swr_free(&swr_context);
    avcodec_close(codec_context);
    avcodec_free_context(&codec_context);
    avformat_close_input(&format);
    avformat_free_context(format);

    //pulse cleaning
    pa_stream_disconnect(stream_pa);
    pa_stream_unref(stream_pa);

    return NULL;
}
