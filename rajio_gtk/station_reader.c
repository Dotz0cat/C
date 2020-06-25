#include <stdlib.h>
#include <stdio.h>

//prototypes
struct station read_file(char* file_name, int station_number, long eof);
long eof_finder(char* file_name);
int append_new_station(char* file_name, struct station s);

struct station {
	int station_number;
	char station_name[100];
	char thumbnail[400];
	int num_of_addresses;
	char address[400];
	char secondary_address[400];
};

struct station read_file(char* file_name, int station_number, long eof) {
	FILE* fp = fopen(file_name, "rb");

	if (!fp) {
		fprintf(stderr, "Threre was a error opening: %s\r\n", file_name);
	}

	float max_stations;

	//get the max number of stations in file
	//this is done by deviding the number of bytes in the file by the size of the stuct that is stored in the file
	max_stations = sizeof(struct station) / eof;

	if (station_number > max_stations) {
		fprintf(stderr, "station is not advaliable");
	}

	float bytes_to_seek;

	bytes_to_seek = station_number * sizeof(struct station);

	fseek(fp, bytes_to_seek, SEEK_SET);

	struct station read_station;

	fread(&read_station, sizeof(struct station), 1, fp);

	return read_station;
}

long eof_finder(char* file_name) {
	FILE* fp = fopen(file_name, "rb");

	if (!fp) {
		fprintf(stderr, "Threre was a error opening: %s\r\n", file_name);
		return -1;
	}

	fseek(fp, 0L, SEEK_END);

	long size = ftell(fp);

	fclose(fp);

	return size;
}

int append_new_station(char* file_name, struct station s) {
	FILE* fp = fopen(file_name, "ab");

	if (!fp) {
		fprintf(stderr, "Threre was a error opening: %s\r\n", file_name);
		return -1;
	}

	fwrite(&s, sizeof(struct station), 1, fp);

	fclose(fp);

	return 0;
}
