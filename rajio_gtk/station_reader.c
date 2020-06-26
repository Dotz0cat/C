#include "rajio.h"

//prototypes 
bson_t* read_file(char* file_name, int station_number);
int append_new_station(char* file_name, bson_t* doc, int number);
int write_to_file(char* file_name, bson_t* doc);


bson_t* read_file(char* file_name, int station_number) {
	bson_t* parent;
	bson_t* doc;

	bson_reader_t* reader;
	reader = bson_reader_new_from_file(file_name, NULL);

	if (!reader) {
		fprintf(stderr, "there was a error opening the bson reader");
	}

	parent = bson_reader_read(reader, NULL);

	if (station_number == 0) {
		bson_reader_destroy(reader);
		return parent;
	}

	bson_iter_t* iter;

	char str[100];

	uint8_t* buffer;

	uint32_t* size;

	sprintf(str, "%i", station_number);

	if (bson_iter_init(iter, doc)) {
		if (strcmp(bson_iter_key(iter), str)) {
			bson_iter_document(iter, size, &buffer);
			doc = bson_new_from_buffer(&buffer, (unsigned long*) size, NULL, NULL);
			bson_reader_destroy(reader);
			return doc;
		}
		while (bson_iter_next(iter)) {
			if (strcmp(bson_iter_key(iter), str)) {
			bson_iter_document(iter, size, &buffer);
			doc = bson_new_from_buffer(&buffer, (unsigned long*) size, NULL, NULL);
			break;
			}
		}
	}

	bson_reader_destroy(reader);
	return doc;
}

int append_new_station(char* file_name, bson_t* doc, int number) {
	bson_t* parent = read_file(file_name, 0);

	char str[100];

	sprintf(str, "%i", number);

	bson_append_document(parent, str, -1, doc);

	if (write_to_file(file_name, parent) < 0) {
		return -1;
	}

	return 0;
}

int write_to_file(char* file_name, bson_t* doc) {
	FILE* fp = fopen(file_name, "wb");

	if (!fp) {
		fprintf(stderr, "Threre was a error opening: %s\r\n", file_name);
		return -1;
	}

	fwrite(doc, sizeof(doc), 1, fp);

	fclose(fp);

	return 0;
}
