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
#include <signal.h>
#include <string.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libswresample/swresample.h>


void print_help(void);
int play_with_libav(char *url);

/*
    my useal test case is http://158.69.38.195:20278
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
    printf("rajio protocol:\\\\url:port\r\n");
}

int play_with_libav(char *url) {
    pa_simple *s;
    pa_sample_spec ss;
    uint8_t* audio_buffer;
    int samples = 512;
    int holder;
    char err[50];

    audio_buffer = malloc(32*1024);

    ss.format = PA_SAMPLE_FLOAT32;
    ss.channels = 2;
    ss.rate = 44100;
    //libav stuff
    avformat_network_init();

    s = pa_simple_new(NULL, "rajio", PA_STREAM_PLAYBACK, NULL, "internet radio", &ss, NULL, NULL, NULL);

    AVFormatContext* format = avformat_alloc_context();


    if ((holder = avformat_open_input(&format, url, NULL, NULL))!=0) {
        fprintf(stderr, "avformat_open_input broke");
        return holder;
    }
    if ((holder = avformat_find_stream_info(format, NULL))<0) {
        fprintf(stderr, "find_stream_info broke");
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



    SwrContext* swr_context = swr_alloc_set_opts(NULL, AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT, AV_SAMPLE_FMT_FLT, (int)ss.rate, (long)codec_context->channel_layout, codec_context->sample_fmt, codec_context->sample_rate, 0, NULL);
    if (!swr_context) {
        fprintf(stderr, "swr broke");
        return -1;
    }
    swr_init(swr_context);

    AVPacket avpkt;
    av_init_packet(&avpkt);
    avpkt.data = NULL;
    avpkt.size = 0;

    AVFrame* frame = av_frame_alloc();

    holder = 0;

    while (av_read_frame(format, &avpkt) >= 0) {
        if (avpkt.stream_index != (int)stream) {
            continue;
        }

        if (avcodec_send_packet(codec_context, &avpkt) < 0) {
            fprintf(stderr, "cant send packet");
            return -1;
        }

        if (avcodec_receive_frame(codec_context, frame) < 0) {
            fprintf(stderr, "receive frame");
            return -1;
        }

        holder = swr_convert(swr_context, &audio_buffer, samples, (const uint8_t**)&frame->data, frame->nb_samples);
        if (holder < 0) {
            fprintf(stderr, "swrconvert not working");
            return -1;
        }

        //printf("%i", holder);

         while (holder>0) {
            pa_simple_write(s, audio_buffer, sizeof(audio_buffer), NULL);

            holder = swr_convert(swr_context, &audio_buffer, samples, NULL, 0);
            if (holder < 0) {
                fprintf(stderr, "swr convert on line 174");
                return -1;
            }
         }

        av_packet_unref(&avpkt);
    }


    printf("going well add more\r\n");

    free(audio_buffer);
    return 0;
}
