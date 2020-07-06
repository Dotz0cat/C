#include "rajio.h"
#include <string.h>
#include <regex.h>

//prototypes
int add_stations(char* file_name, char* sql_file);
int m3u_parser(char* file_name, char* sql_file);
int pls_parser(char* file_name, char* sql_file);
int is_valid_url(char* url);

//external prototypes
extern int append_new_address(char* file_name, int id, char* address);
extern int get_highest_id(char* file_name);

int add_stations(char* file_name, char* sql_file) {
	char string[512];

	strcpy(string, file_name);

	char* token;

	token = strtok(string, ".");

	//check file endings
	//it checks in case of several '.'s
	//next i hope to make it check for magic stuff
	while (token != NULL) {
		//else if trees fun
		if (strcmp(token, "m3u") == 0) {
			return m3u_parser(file_name, sql_file);
		}
		else if (strcmp(token, "pls") == 0) {
			return pls_parser(file_name, sql_file);
		}
		else if (strcmp(token, "m3u8") == 0) {
			return m3u_parser(file_name, sql_file);
		}


		//rerun token in case this is not the only '.'
		token = strtok(NULL, ".");
	}

	//do magic checking here

	//return 0 casuse it had no addresses
	return 0;
}

int m3u_parser(char* file_name, char* sql_file) {
	FILE* fp = fopen(file_name, "r");

	if (!fp) {
		fprintf(stderr, "cannot open: %s\r\n", file_name);

		return -1;
	}

	int addresses = 0;

	char str[512];
	int id = get_highest_id(sql_file) + 1;

	//regex stuff
	regex_t regex;
	regex_t comment_regex;
	int error_num;
	char err_msg[256];

	//old regex [(http(s)?):\/\/(www\.)?a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,6}\b([-a-zA-Z0-9@:%_\+.~#?&//=]*)
	//tried ^(http:\/\/www\.|https:\/\/www\.|http:\/\/|https:\/\/)[a-z0-9]+([\-\.]{1}[a-z0-9]+)*\.[a-z]{2,5}(:[0-9]{1,5})?(\/.*)?$ 

	error_num = regcomp(&regex, "^\\(http\\|ftp\\|http\\):\\/\\/\\(www\\.\\)\\?[a-z0-9A-z\\.:]\\{2,256\\}", REG_ICASE);
	if (error_num) {
		regerror(error_num, &regex, err_msg, sizeof(err_msg));
		fprintf(stderr, "error doing regex: %s\n", err_msg);
		fclose(fp);
		return -1;
	}

	error_num = regcomp(&comment_regex, "^#", 0);
	if (error_num) {
		regerror(error_num, &comment_regex, err_msg, sizeof(err_msg));
		fprintf(stderr, "error doing regex: %s\n", err_msg);
		regfree(&regex);
		fclose(fp);
		return -1;
	}
	
	int comment;

	while (fgets(str, (int) sizeof(str), fp) != NULL) {
		comment = 0;

		error_num = regexec(&comment_regex, str, 0, NULL, 0);
		if(!error_num) {
			comment = 1;
		}
			

		if (comment == 0) {
			error_num = regexec(&regex, str, 0, NULL, 0);
			printf("%s\r\n", str);
			if(!error_num) {
				addresses++;
				if (append_new_address(sql_file, id, str) != 0) {
					fprintf(stderr, "there was a error appending %s to sql file\r\n", str);
					regfree(&regex);
					regfree(&comment_regex);
					fclose(fp);
					return -1;
				}
			}
			/*else {
				regerror(error_num, &comment_regex, err_msg, sizeof(err_msg));
				fprintf(stderr, "error doing regex: %s\n", err_msg);
			}*/
		}
	}

	regfree(&regex);
	regfree(&comment_regex);

	fclose(fp);

	return addresses;
}

int pls_parser(char* file_name, char* sql_file) {
	FILE* fp = fopen(file_name, "r");

	if (!fp) {
		fprintf(stderr, "cannot open: %s\r\n", file_name);

		return -1;
	}

	int addresses = 0;

	//it just looks better to me
	char str[512];
	int id = get_highest_id(sql_file) + 1;

	//regex stuff
	regex_t regex;
	regex_t file_regex;
	regex_t header_regex;
	regex_t http;
	regex_t https;
	regex_t ftp;
	int error_num;
	char err_msg[256];

	error_num = regcomp(&regex, "\\(http\\|ftp\\|http\\):\\/\\/\\(www\\.\\)\\?[a-z0-9A-z\\.:]\\{2,256\\}", REG_ICASE);
	if (error_num) {
		regerror(error_num, &regex, err_msg, sizeof(err_msg));
		fprintf(stderr, "error doing regex: %s\n", err_msg);
		fclose(fp);
		return -1;
	}

	error_num = regcomp(&file_regex, "^file[0-9]\\{1,4\\}=", REG_ICASE);
	if (error_num) {
		regerror(error_num, &file_regex, err_msg, sizeof(err_msg));
		fprintf(stderr, "error doing regex: %s\n", err_msg);
		regfree(&regex);
		fclose(fp);
		return -1;
	}

	error_num = regcomp(&header_regex, "\\[playlist\\]", REG_ICASE);
	if (error_num) {
		regerror(error_num, &header_regex, err_msg, sizeof(err_msg));
		fprintf(stderr, "error doing regex: %s\n", err_msg);
		regfree(&regex);
		regfree(&file_regex);
		fclose(fp);
		return -1;
	}

	error_num = regcomp(&http, "http:\\/\\/", REG_ICASE);
	if (error_num) {
		regerror(error_num, &header_regex, err_msg, sizeof(err_msg));
		fprintf(stderr, "error doing regex: %s\n", err_msg);
		regfree(&regex);
		regfree(&file_regex);
		regfree(&header_regex);
		fclose(fp);
		return -1;
	}

	error_num = regcomp(&https, "https:\\/\\/", REG_ICASE);
	if (error_num) {
		regerror(error_num, &header_regex, err_msg, sizeof(err_msg));
		fprintf(stderr, "error doing regex: %s\n", err_msg);
		regfree(&regex);
		regfree(&file_regex);
		regfree(&header_regex);
		regfree(&http);
		fclose(fp);
		return -1;
	}

	error_num = regcomp(&ftp, "ftp:\\/\\/", REG_ICASE);
	if (error_num) {
		regerror(error_num, &header_regex, err_msg, sizeof(err_msg));
		fprintf(stderr, "error doing regex: %s\n", err_msg);
		regfree(&regex);
		regfree(&file_regex);
		regfree(&header_regex);
		regfree(&http);
		regfree(&https);
		fclose(fp);
		return -1;
	}

	int header_found = 0;

	fgets(str, (int) sizeof(str), fp);

	error_num = regexec(&header_regex, str, 0, NULL, 0);
	if (!error_num) {
		header_found = 1;
	}

	while (fgets(str, (int) sizeof(str), fp) != NULL) {
		if (header_found == 1) {
			error_num = regexec(&file_regex, str, 0, NULL, 0);
			if (!error_num) {
				error_num = regexec(&regex, str, 0, NULL, 0);
				if (!error_num) {
					
					if (regexec(&http, str, 0, NULL, 0) == 0) {
						char* string;
						string = strstr(str, "http");
						addresses++;
						if (append_new_address(sql_file, id, string) != 0) {
							fprintf(stderr, "there was a error appending %s to sql file\r\n", str);
							regfree(&regex);
							regfree(&file_regex);
							regfree(&header_regex);
							regfree(&http);
							regfree(&https);
							regfree(&ftp);
							fclose(fp);
							return -1;
						}
					}
					else if (regexec(&https, str, 0, NULL, 0) == 0) {
						char* string;
						string = strstr(str, "https");
						addresses++;
						if (append_new_address(sql_file, id, string) != 0) {
							fprintf(stderr, "there was a error appending %s to sql file\r\n", str);
							regfree(&regex);
							regfree(&file_regex);
							regfree(&header_regex);
							regfree(&http);
							regfree(&https);
							regfree(&ftp);
							fclose(fp);
							return -1;
						}
					}
					else if (regexec(&ftp, str, 0, NULL, 0) == 0) {
						char* string;
						string = strstr(str, "ftp");
						addresses++;
						if (append_new_address(sql_file, id, string) != 0) {
							fprintf(stderr, "there was a error appending %s to sql file\r\n", str);
							regfree(&regex);
							regfree(&file_regex);
							regfree(&header_regex);
							regfree(&http);
							regfree(&https);
							regfree(&ftp);
							fclose(fp);
							return -1;
						}
					}
				}
			}
		}
	}

	regfree(&regex);
	regfree(&file_regex);
	regfree(&header_regex);
	regfree(&http);
	regfree(&https);
	regfree(&ftp);

	fclose(fp);

	return addresses;
}

//returns 0 if it is a valid url
int is_valid_url(char* url) {
	regex_t regex;
	int error_num;
	char err_msg[256];

	error_num = regcomp(&regex, "^\\(http\\|ftp\\|http\\):\\/\\/\\(www\\.\\)\\?[a-z0-9A-z\\.:]\\{2,256\\}", REG_ICASE);
	if (error_num) {
		regerror(error_num, &regex, err_msg, sizeof(err_msg));
		fprintf(stderr, "error doing regex: %s\r\n", err_msg);
		return -1;
	}

	error_num = regexec(&regex, url, 0, NULL, 0);
	if (!error_num) {
		regfree(&regex);
		return 0;
	}

	regfree(&regex);

	return 1;
}
