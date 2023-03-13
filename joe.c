/*
Copyright 2023 Dotz0cat

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*/

/*
cc joe.c -o joe
*/

#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else 
#include <unistd.h>
#endif

enum state {
	Alive,
	Dead
};

struct things {
	int number;
	enum state status;

	void* next;
};

void print_status(int number, struct things *list, int verbose);
#ifdef _WIN32
int clear(void);
#endif

int main(int argc, char** argv) {
	int verbose = 0;
	int number = 1;
	int k_number = 2;

//if win32 is not defined
#ifndef _WIN32
	int opt;
	while((opt = getopt(argc, argv, "vVn:k:")) != -1) {
        switch(opt) {
            case 'v':
                verbose = 1;
                break;
            case 'V':
                verbose = 2;
                break;
            case 'n':
                number = atoi(optarg);
                break;
            case 'k':
                k_number = atoi(optarg);
                break;
            default: /* ? */
                fprintf(stderr, "Usage: %s [-v] [-n n] [-k k]\r\n", argv[0]);
                return EXIT_FAILURE;
        }
    }
#endif

#ifdef __WIN32
    for (int i = 0; i < argc; i++) {
    	if (argv[i][0] == '-') {
    		switch (argv[i][1]) {
    			case 'v':
	                verbose = 1;
	                break;
	            case 'V':
	                verbose = 2;
	                break;
	            case 'n':
	            	if ((i + 1) >= argc) {
	            		fprintf(stderr, "%s: -n requires an argument\r\n", argv[0]);
	            		return EXIT_FAILURE;
	            	}
	            	if (!atoi(argv[i + 1])) {
	            		fprintf(stderr, "%s: -n requires an argument\r\n", argv[0]);
	            		return EXIT_FAILURE;
	            	}
	                number = atoi(argv[i + 1]);
	                break;
	            case 'k':
	            	if ((i + 1) >= argc) {
	            		fprintf(stderr, "%s: -k requires an argument\r\n", argv[0]);
	            		return EXIT_FAILURE;
	            	}
	            	if (!atoi(argv[i + 1])) {
	            		fprintf(stderr, "%s: -k requires an argument\r\n", argv[0]);
	            		return EXIT_FAILURE;
	            	}
	                k_number = atoi(argv[i + 1]);
	                break;
	            case '\0':
	            default: /* ? */
	                fprintf(stderr, "Usage: %s [-v] [-n n] [-k k]\r\n", argv[0]);
	                return EXIT_FAILURE;
    		}
    	}
    	else if (argv[i][0] == '/') {
    		switch (argv[i][1]) {
    			case 'v':
	                verbose = 1;
	                break;
	            case 'V':
	                verbose = 2;
	                break;
	            case 'n':
	            	if ((i + 1) >= argc) {
	            		fprintf(stderr, "%s: -n requires an argument\r\n", argv[0]);
	            		return EXIT_FAILURE;
	            	}
	            	if (!atoi(argv[i + 1])) {
	            		fprintf(stderr, "%s: -n requires an argument\r\n", argv[0]);
	            		return EXIT_FAILURE;
	            	}
	                number = atoi(argv[i + 1]);
	                break;
	            case 'k':
	            	if ((i + 1) >= argc) {
	            		fprintf(stderr, "%s: -k requires an argument\r\n", argv[0]);
	            		return EXIT_FAILURE;
	            	}
	            	if (!atoi(argv[i + 1])) {
	            		fprintf(stderr, "%s: -k requires an argument\r\n", argv[0]);
	            		return EXIT_FAILURE;
	            	}
	                k_number = atoi(argv[i + 1]);
	                break;
	            case '\0':
	            default: /* ? */
	                fprintf(stderr, "Usage: %s [-v] [-n n] [-k k]\r\n", argv[0]);
	                return EXIT_FAILURE;
    		}
    	}
    }
#endif

    if (number <= 0) {
    	fprintf(stderr, "n must be greater than 0\r\n");
    	return 1;
    }

    if (k_number <= 1) {
    	fprintf(stderr, "k must be greater than 1\r\n");
    	return 1;
    }


	struct things *list = malloc(sizeof(struct things) * number);
	int alive = number;

	for (int i = 0; i <= number; i++) {
		list[i].number = i + 1;
		list[i].status = Alive;
		if (i != 0 || i != number) {
			list[i - 1].next = &list[i];
		}
		
	}
	list[0].next = &list[1];
	list[number - 1].next = &list[0];
	
	struct things *head = list;
	struct things *current = head;

#ifdef _WIN32
	clear();
#else
	printf("\033c");
#endif

	if (verbose) {
		print_status(number, list, verbose);
	}

	while (alive != 1) {

		//skip ahead by k
		if (alive == number) {
			for (int i = 1; i < k_number; i++) {
				current = current->next;
			}
		}
		else {
			for (int i = 1; i < k_number;) {
				if (current->status == Alive) {
					i++;
				}
				current = current->next;
			}
		}
		
		if (current->status == Dead) {
			do {
				current = current->next;
			} while (current->status == Dead);
		}
		current->status = Dead;
		alive--;

		if (verbose) {
			print_status(number, list, verbose);
#ifdef _WIN32
			//milliseconds
			Sleep(1000);
#else
			sleep(1);
#endif
		}
	}

	print_status(number, list, verbose);

	return 0;
}

void print_status(int number, struct things *list, int verbose) {
	if (verbose == 1) {
#ifdef _WIN32
		clear();
#else
		printf("\033c");
#endif
	}

	for (int i = 0; i < number; i++) {
		if (list[i].status == Alive) {
			printf("%4i ", list[i].number);
		}
		else {
			printf("dead ");
		}
		
	}
	printf("\r\n");
}

#ifdef _WIN32
//from https://learn.microsoft.com/en-us/windows/console/clearing-the-screen
int clear(void) {
    HANDLE hStdOut;

    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // Fetch existing console mode so we correctly add a flag and not turn off others
    DWORD mode = 0;
    if (!GetConsoleMode(hStdOut, &mode)) {
        return GetLastError();
    }

    // Hold original mode to restore on exit to be cooperative with other command-line apps.
    const DWORD originalMode = mode;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    // Try to set the mode.
    if (!SetConsoleMode(hStdOut, mode)) {
        return GetLastError();
    }

    // Write the sequence for clearing the display.
    DWORD written = 0;
    PCWSTR sequence = L"\ec";
    if (!WriteConsoleW(hStdOut, sequence, (DWORD)wcslen(sequence), &written, NULL)) {
        // If we fail, try to restore the mode on the way out.
        SetConsoleMode(hStdOut, originalMode);
        return GetLastError();
    }

    // To also clear the scroll back, emit L"\x1b[3J" as well.
    // 2J only clears the visible window and 3J only clears the scroll back.

    // Restore the mode on the way out to be nice to other command-line applications.
    SetConsoleMode(hStdOut, originalMode);

    return 0;
}
#endif
