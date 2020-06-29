#include "rajio.h"

//prototypes 
int append_new_station(char* file_name, int id, char* name, char* thumbnail, int num_of_addresses);
int append_new_address(char* file_name, int id, char* address);

int append_new_station(char* file_name, int id, char* name, char* thumbnail, int num_of_addresses) {
	sqlite3* db;
	sqlite3_stmt* stmt;

	int rc = sqlite3_open(file_name, &db);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "cannot open %s\r\n", file_name);

		sqlite3_close(db);

		return -1;
	}

	char* sql = "INSERT INTO Stations VALUES(Id=?, Name=?, Thumbnail=?, Num_of_addresses=?);";

	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

	if (rc == SQLITE_OK) {
		sqlite3_bind_int(stmt, 1, id);
		sqlite3_bind_text(stmt, 2, name, -1, NULL);
		sqlite3_bind_text(stmt, 3, thumbnail, -1, NULL);
		sqlite3_bind_int(stmt, 4, num_of_addresses);
	}
	else {
		//think of a good error message

		sqlite3_close(db);

		return -1;
	}

	rc = sqlite3_step(stmt);

	/*if (rc != SQLITE_DONE) {
		//think of good error message

		sqlite3_close(db);

		return -1;
	}*/

	sqlite3_finalize(stmt);

	sqlite3_close(db);

	return 0;
}

int append_new_address(char* file_name, int id, char* address) {
	sqlite3* db;

	sqlite3_stmt* stmt;

	int rc = sqlite3_open(file_name, &db);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "cannot open %s\r\n", file_name);

		sqlite3_close(db);

		return -1;
	}

	char* sql = "INSERT INTO Adresses VALUES(Id=?, Address=?);";

	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

	if (rc == SQLITE_OK) {
		sqlite3_bind_int(stmt, 1, id);
		sqlite3_bind_text(stmt, 2, address, -1, NULL);
	}
	else {
		//think of a good error message

		sqlite3_close(db);

		return -1;
	}

	rc = sqlite3_step(stmt);

	/*if (rc != SQLITE_DONE) {
		//think of good error message

		sqlite3_close(db);

		return -1;
	}*/

	sqlite3_finalize(stmt);

	sqlite3_close(db);

	return 0;
}
