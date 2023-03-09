#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

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

int main(int argc, char** argv) {
	int verbose = 0;
	int number = 1;
	int k_number = 2;

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
                fprintf(stderr, "Usage: %s [-v] [-n] [-k]\r\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

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

	printf("\033c");

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
			sleep(1);
		}
	}

	print_status(number, list, verbose);

	return 0;
}

void print_status(int number, struct things *list, int verbose) {
	if (verbose == 1) {
		printf("\033c");
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
