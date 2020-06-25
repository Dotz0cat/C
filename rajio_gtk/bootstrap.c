#include <stdlib.h>
#include <stdio.h>

struct station {
	int station_number;
	char station_name[100];
	char thumbnail[400];
	int num_of_addresses;
	char address[400];
	char secondary_address[400];
};

extern int append_new_station(char* file_name, struct station s);

int main() {

	struct station write;

	char buffer[400];
	printf("What is the number of the station?");
	fgets(buffer, sizeof(buffer), stdin);
	sscanf(buffer, "%i", &write.station_number);

	printf("What is the name of the station?");
	fgets(buffer, sizeof(buffer), stdin);
	sscanf(buffer, "%s", write.station_name);

	printf("Where is the thumbnail for the station?");
	fgets(buffer, sizeof(buffer), stdin);
	sscanf(buffer, "%s", write.thumbnail);

	printf("How many addresess does the station have?");
	fgets(buffer, sizeof(buffer), stdin);
	sscanf(buffer, "%i", &write.num_of_addresses);

	printf("What is the first address?");
	fgets(buffer, sizeof(buffer), stdin);
	sscanf(buffer, "%s", write.address);

	printf("What is the seconary address?");
	fgets(buffer, sizeof(buffer), stdin);
	sscanf(buffer, "%s", write.secondary_address);

	if (append_new_station("/home/seth/c/C/rajio_gtk/stations", write) < 0) {
		fprintf(stderr, "There was a error appending to the file");
		return -1;
	}


	return 0;
}
