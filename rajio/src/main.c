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
//static void clean_up(pa_simple *s, int socket_num);
//static void sigint_catcher();


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

void print_help() {
    printf("the proper way to use this program at the moment is with the syantax\r\n");
    printf("rajio ip_of_station port\r\n");
    printf("or rajio url port\r\nthat is when i figure out dns\r\n");
}

int socket_stuff(char* ip, int port) {
    //make socket
        int socket_num;
        int connection;
        char get[100];
        int played;

        //makes the get request
        //need to get this fixxed it is not protected
        strcat(get, "GET / HTTP/1.1\r\nHost: ");
        strcat(get, ip);
        strcat(get, ":");
        strcat(get, "20278");
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

int play_audio(int socket_num) {
    pa_simple *s;
    pa_sample_spec ss;
    char network_buffer[5120];
    char audio_buffer[5120];
    int go = 1;
    int holder;

    ss.format = PA_SAMPLE_S16NE;
    ss.channels = 2;
    ss.rate = 44100;
    //libav stuff
    av_register_all();

    s = pa_simple_new(NULL, "rajio", PA_STREAM_PLAYBACK, NULL, "internet radio", &ss, NULL, NULL, NULL);

    AVFormatContext* format = avformat_alloc_context();
    //run first recive outside of loop then start looping
    recv(socket_num, network_buffer, sizeof(network_buffer), 0);
    recv(socket_num, network_buffer, sizeof(network_buffer), 0);
    if ((holder = avformat_open_input(&format, network_buffer, NULL, NULL))==-1) {
        return holder;
    }
    if ((holder = avformat_find_stream_info(format, NULL))==-1) {
        return holder;
    }


    //need to figure out how to buffer the data from the socket and then play it while refilling the buffer
    while (go==1) {
        recv(socket_num, network_buffer, sizeof(network_buffer), 0);
        pa_simple_write(s, network_buffer, sizeof(network_buffer), NULL);
    }
    return 0;
}

/*
void clean_up(pa_simple *s, int socket_num) {
    pa_simple_flush(s,NULL);

    close(socket_num);
}

void sigint_catcher() {
    //dont know how to get values for clean_up()
    clean_up();
}
*/
