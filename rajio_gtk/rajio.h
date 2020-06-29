#include <stdlib.h>
#include <stdio.h>
#include <libbson-1.0/bson/bson.h>
#include <sqlite3.h>

struct station {
    int station_number;
    char station_name[100];
    char thumbnail[400];
    int num_of_addresses;
    char address[400];
    char secondary_address[400];
};


/*
	playing here
	what if i have 2 tables a address table and a table for everything else?
	
*/
