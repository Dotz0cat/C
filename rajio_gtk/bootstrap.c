#include "rajio.h"

extern int append_new_station(char* file_name, bson_t* doc, int number);
extern int write_to_file(char* file_name, bson_t* doc);
void first_run(char* file_name);

int main(int argc, char* argv[]) {

	if (argc >= 1) {
		//assume it is the first run
		first_run("/home/seth/c/C/rajio_gtk/stations");
	}

	bson_t doc;

	bson_init(&doc);


	char buffer[400];
	char str[400];
	int num;
	int addresess;

	printf("What is the number of the station?");
	fgets(buffer, sizeof(buffer), stdin);
	sscanf(buffer, "%i", &num);
	bson_append_int32(&doc, "number", -1, (int32_t)num);

	printf("What is the name of the station?");
	fgets(buffer, sizeof(buffer), stdin);
	sscanf(buffer, "%s", str);
	bson_append_utf8(&doc, "name", -1, str, -1);

	printf("Where is the thumbnail for the station?");
	fgets(buffer, sizeof(buffer), stdin);
	sscanf(buffer, "%s", str);
	bson_append_utf8(&doc, "thumbnail", -1, str, -1);

	printf("How many addresess does the station have?");
	fgets(buffer, sizeof(buffer), stdin);
	sscanf(buffer, "%i", &addresess);
	bson_append_int32(&doc, "num_of_statons", -1, (int32_t) addresess);

	if(addresess==0) {
		return -1;
	}

	char name[100];
	char address_num[100];

	for (int i = 0; i < addresess; i++) {
		printf("Please enter an address to the station");
		fgets(buffer, sizeof(buffer), stdin);
		sscanf(buffer, "%s", str);
		strcat(name, "address");
		sprintf(address_num, "%i", i);
		strcat(name, address_num);
		bson_append_utf8(&doc, name, -1, str, -1);
	}


	if (append_new_station("/home/seth/c/C/rajio_gtk/stations", &doc, num) < 0) {
		fprintf(stderr, "There was a error appending to the file");
		return -1;
	}


	return 0;
}

void first_run(char* file_name) {
	bson_t* parent;
	//bson_init(parent);
	parent = bson_new();
	write_to_file(file_name, parent);
	bson_free(parent);
}
