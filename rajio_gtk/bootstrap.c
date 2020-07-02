#include "rajio.h"

extern int append_new_station(char* file_name, int id, char* name, char* thumbnail, int num_of_addresses);
extern int append_new_address(char* file_name, int id, char* address);

int main(int argc, char* argv[]) {

	if (argc > 1) {
		//assume it is the first run

		sqlite3* db;

		int rc = sqlite3_open("/home/seth/c/C/rajio_gtk/stations", &db);

		if (rc != SQLITE_OK) {
		fprintf(stderr, "cannot open %s\r\n", "/home/seth/c/C/rajio_gtk/stations");

		sqlite3_close(db);

		return -1;
		}

		char* sql = "CREATE TABLE Stations (Id INT, Name TEXT, Thumbnail TEXT, Num_of_addresses INT);"
					"CREATE TABLE Addresses (Id INT, Address TEXT);";

		rc = sqlite3_exec(db, sql, NULL, NULL, NULL);

		if (rc != SQLITE_OK) {
			//good error message here

			sqlite3_close(db);

			return -1;
		}

		//on to the main bootstraper

	}

	char buffer[400];
	char str[400];
	int num;
	char name[100];
	char thumbnail[400];
	int addresess;

	printf("What is the number of the station? ");
	fgets(buffer, sizeof(buffer), stdin);
	sscanf(buffer, "%i", &num);

	printf("What is the name of the station? ");
	fgets(buffer, sizeof(buffer), stdin);
	sscanf(buffer, "%s", name);

	printf("Where is the thumbnail for the station? ");
	fgets(buffer, sizeof(buffer), stdin);
	sscanf(buffer, "%s", thumbnail);

	printf("How many addresess does the station have? ");
	fgets(buffer, sizeof(buffer), stdin);
	sscanf(buffer, "%i", &addresess);

	append_new_station("/home/seth/c/C/rajio_gtk/stations", num, name, thumbnail, addresess);

	if(addresess==0) {
		return -1;
	}

	for (int i = 0; i < addresess; i++) {
		printf("Please enter an address to the station: ");
		fgets(buffer, sizeof(buffer), stdin);
		sscanf(buffer, "%s", str);
		
		append_new_address("/home/seth/c/C/rajio_gtk/stations", num, str);
	}

	return 0;
}
