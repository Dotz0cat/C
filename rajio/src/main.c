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

//prototypes
static void print_help();
static int socket_stuff(char* ip, int port);
static int play_audio(int socket_num);

static int read_packet(void *opaque, uint8_t *buf, int buf_size);


struct buffer_data {
    uint8_t *ptr;
    size_t size;
};

/*
    My test case for this program is 158.69.38.195 port 20278
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
        case (3):
            #ifdef DEBUG
                printf(argv[1]);
            #endif
            success = socket_stuff(argv[1], atoi(argv[2]));
            return success;
        default:
            print_help();
            return 0;
        }
    }
    success = 0;

    return success;
}

static void print_help() {
    printf("the proper way to use this program at the moment is with the syantax\r\n");
    printf("rajio ip_of_station port\r\n");
    printf("or rajio url port\r\nthat is when i figure out dns\r\n");
}

static int socket_stuff(char* ip, int port) {
    //make socket
        int socket_num;
        int connection;
        char get[100];
        int played;
        char port_str[10];

        //converts port to string
        //fprintf(port_str, "%i", port);

        //makes the get request
        //need to get this fixxed it is not protected
        strcat(get, "GET / HTTP/1.1\r\nHost: ");
        strcat(get, ip);
        strcat(get, ":");
        strcat(get, port_str);
        //hardcoded just to attemp to test it
        strcat(get, "\r\nAccept: */*\r\n\r\n");

        socket_num = socket(AF_INET, SOCK_STREAM, 0);

        //connect
        struct sockaddr_in server_addres;
        server_addres.sin_family = AF_INET;
        server_addres.sin_port = htons(port);
        int some = inet_aton(ip, &server_addres.sin_addr);
        if (some==0) {
            fprintf(stderr, "ip is not valid\r\n");
            return some;
        }
        connection = connect(socket_num, (struct sockaddr*)&server_addres, sizeof(server_addres));
        if (connection!=0) {
            fprintf(stderr, "The ip adress is not valid or the connection failed\r\n");
            return connection;
        }

        //plays audio
        //need to add a get request
        send(socket_num, get, sizeof(get), 0);
        played = play_audio(socket_num);
        return played;
}

static int play_audio(int socket_num) {
    pa_simple *s;
    pa_sample_spec ss;
    uint8_t* network_buffer;
    struct buffer_data buf;
    //char audio_buffer[5120];
    int go = 1;
    int holder;

    ss.format = PA_SAMPLE_S16NE;
    ss.channels = 2;
    ss.rate = 44100;
    //libav stuff
    av_register_all();

    s = pa_simple_new(NULL, "rajio", PA_STREAM_PLAYBACK, NULL, "internet radio", &ss, NULL, NULL, NULL);


    //memalloc network_buffer to stop a segfault
    network_buffer = (uint8_t*)malloc((12*1024) * sizeof(uint8_t));


    recv(socket_num, network_buffer, sizeof(network_buffer), 0);

    buf.ptr = network_buffer;
    buf.size = sizeof(network_buffer);
    /**/
    //stuff i wish i could put elsewhere
    int buffer_size = 12 * 1024;
    unsigned char* pbuffer;

    //attempting to fix a bad free
    pbuffer = (unsigned char*)malloc(buffer_size * sizeof(unsigned char));

    //i dont even understand half of this
    AVIOContext* pIOCtx = avio_alloc_context(pbuffer, buffer_size, 0, &buf, &read_packet, NULL, NULL);

    AVFormatContext* pCtx = avformat_alloc_context();

    pCtx ->pb = pIOCtx;

    pCtx->avio_flags = AVFMT_FLAG_CUSTOM_IO;


    /**/

    if ((holder = avformat_open_input(&pCtx, "", NULL, NULL))<0) {
        return holder;
    }
    if ((holder = avformat_find_stream_info(pCtx, NULL))<0) {
        return holder;
    }

    size_t stream = 0;

    for (; stream < pCtx->nb_streams; stream++) {
        if (pCtx->streams[stream]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            continue;
        }
    }

    AVCodecContext* codec_context = pCtx->streams[stream]->codec;

    if (codec_context->channel_layout == 0) {
        codec_context->channel_layout = AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT;
    }

    AVCodec* codec1 = avcodec_find_decoder(codec_context->codec_id);

    //need to figure out how to buffer the data from the socket and then play it while refilling the buffer
    while (go==1) {
        recv(socket_num, network_buffer, sizeof(network_buffer), 0);
        pa_simple_write(s, network_buffer, sizeof(network_buffer), NULL);
    }
    return 0;
}

//shamelessly stolen from http://www.ffmpeg.org/doxygen/trunk/avio_reading_8c-example.html
static int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    struct buffer_data *bd = (struct buffer_data *)opaque;
    buf_size = FFMIN(buf_size, bd->size);
    if (!buf_size)
        return AVERROR_EOF;
    printf("ptr:%p size:%zu\n", bd->ptr, bd->size);
    /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr  += buf_size;
    bd->size -= buf_size;
    return buf_size;
}


