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

//prototypes
static void print_help();
static int socket_stuff(char* ip, int port);
static void play_audio(int socket_num);
static void clean_up(pa_simple *s, int socket_num);
static void sigint_catcher()



int main(int argc, char *argv[])
{
    // print help if -h
    if (argc==0) {
       print_help();
       return 0;
    }
    else if (argc!=0) {
        switch (argc) {
        case (1):
            print_help();
            return 0;
            break;
        case (2):
            if ((fnmatch(argv[0], "+.+.+.+")==0)) {
                #ifdef DEBUG:
                    printf(argv[0]);
                #endif
                if (atoi(argv[1])!=0) {
                    int success = socket_stuff(argv[0], atoi(argv[1]));
                    return success;
                }
            }
        default:
            print_help();
            return 0;
        }
    }

    return 0;
}

void print_help() {
    printf("the proper way to use this program at the moment is with the syantax\r\n");
    printf("rajio ip_of_station port\r\n");
    printf("or rajio url port\r\nthat is when i figure out dns\r\n");
}

int socket_stuff(char* ip, int port) {
    //make socket
        int socket;
        int connection;
        char get[100];

        //makes the get request
        //need to get this fixxed it is not protected
        strcat(get, "GET / HTTP/1.1\r\nHost: ");
        strcat(get, ip);
        strcat(get, ":");
        strcat(get, port);
        strcat(get, "\r\nAccept: */*");

        socket = socket(AF_INET, SOCK_STREAM, 0);

        //connect
        struct sockaddr_in server_addres;
        server_addres.sin_family = AF_INET;
        server_addres.sin_port = htons(port);
        inet_aton(ip, server_addres.sin_addr);
        connection = connect(socket, (sockaddr *) &server_addres, sizeof(server_addres));
        if (connection!=0) {
            fprintf(stderr, "The ip adress is not valid or the connection failed\r\n");
            return connection;
        }

        //plays audio
        //need to add a get request

        play_audio(socket);

}

void play_audio(int socket_num) {
    pa_simple *s;
    pa_sample_spec ss;

    ss.format = PA_SAMPLE_S16NE;
    ss.channels = 2;
    ss.rate = 44100;

    s = pa_simple_new(NULL, "rajio", PA_STREAM_PLAYBACK, NULL, "internet radio", &ss, NULL, NULL, NULL);
    //need to figure out how to buffer the data from the socket and then play it while refilling the buffer

}

void clean_up(pa_simple *s, int socket_num) {
    pa_simple_flush(s,NULL);

    close(socket_num);
}

void sigint_catcher() {
    //dont know how to get values for clean_up()
    clean_up();
}

