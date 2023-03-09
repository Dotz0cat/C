#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

enum state {
	alive,
	dead
};

struct things {
	int number;
	enum state status;

	void* next;
};

void print_so_far(int number, struct things *list);
void print_final(int number, struct things *list);

int main(int argc, char** argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s n\r\n", argv[0]);
		return 1;
	}

	int number = atoi(argv[1]);
	struct things *list = malloc(sizeof(struct things) * number);
	int alive = number;

	for (int i = 0; i <= number; i++) {
		list[i].number = i + 1;
		list[i].status = alive;
		if (i != 0 || i != number) {
			list[i - 1].next = &list[i];
		}
		
	}
	list[0].next = &list[1];
	list[number - 1].next = &list[0];
	
	struct things *head = list;
	struct things *current = head;
	

	// do {
	int last = 0;
	while (alive != 1 || alive <= 0) {
		//skip ahead by 6
		for (int i = 0; i < 6; i++) {
			current = current->next;
		}
		if (current->number < last) {
			print_final(number, list);
			sleep(5);
		}
		if (current->status == dead) {
			do {
				current = current->next;
			} while (current->status == dead);
		}
		current->status = dead;
		alive--;
		last = current->number;

		if (alive == 1) {
			break;
		}

	}

	print_final(number, list);

	// } while (1);

	return 0;
}

void print_so_far(int number, struct things *list) {
	printf("\033c");
	for (int i = 0; i < number; i++) {
		if (list[i].status == alive) {
			printf("%4i", list[i].number);
		}
		
	}
	printf("\r\n");
}

void print_final(int number, struct things *list) {
	printf("\033c");
	for (int i = 0; i < number; i++) {
		if (list[i].status == alive) {
			printf("%4i", list[i].number);
		}
		else {
			printf("dead ");
		}
		
	}
	printf("\r\n");
}
