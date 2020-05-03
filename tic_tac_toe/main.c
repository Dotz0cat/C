#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>
#include <time.h>
#define board_size 3

static void play(char board[][board_size], char X_or_O);
static void com_play(char board[][board_size], char X_or_O);
static void grade(char board[][board_size], char X_or_O);
static int pre_grade(char board[][board_size], char X_or_O);
static void print_board(char board[][board_size]);
static int check(char board[][board_size], char XO);
static int check_tie(char board[][board_size]);


int main()
{
    char X_or_O;
    int go = 0;
    char board[board_size][board_size] = {
        {'*','*','*'},
        {'*','*','*'},
        {'*','*','*'}
    };
    char buffer[50];

    //hard coded so gcc does not yell at me
    X_or_O = 'X';

    while (go==0) {
        printf("Xs or Os?\r\n");
        (void) fgets(buffer, (int)sizeof(buffer), stdin);
        (void) sscanf(buffer, "%c", &X_or_O);
        if (X_or_O=='X'||X_or_O=='x') {
            printf("You have Chosen Xs\r\n");
            X_or_O = 'X';
            go = 1;
        }
        else if (X_or_O=='O' || X_or_O=='o') {
            printf("You have chosen Os\r\n");
            X_or_O = 'O';
            go = 1;
        }
        else {
            printf("Please enter X or O\r\n");
        }
    }
    play(board, X_or_O);
    return 0;
}

void play(char board[][board_size], char X_or_O) {
    int i, j, play_valid;
    int go = 0;
    char place_to_play[50];
    char buffer[50];
    i = 0;
    j = 0;

    printf("Player's move:\r\n");
    print_board(board);

    while (go==0) {
        printf("Where do you want to play?\r\nFormat(x,y)\r\n");
        (void) fgets(buffer, (int)sizeof(buffer), stdin);
        (void) sscanf(buffer, "%s", place_to_play);
        play_valid = fnmatch("(?,?)", place_to_play, 0);
        if (play_valid==0) {
            switch (place_to_play[1]) {
            case ('1'):
                i = 1;
                break;
            case ('2'):
                i = 2;
                break;
            case ('3'):
                i = 3;
                break;
            default:
                printf("%c is not valid\r\n", place_to_play[1]);
            }
            switch (place_to_play[3]) {
            case ('1'):
                j = 1;
                break;
            case ('2'):
                j = 2;
                break;
            case ('3'):
                j = 3;
                break;
            default:
                printf("%c is not valid\r\n", place_to_play[3]);
            }
            /*checks if the spot is vaild to play*/
            if (board[i-1][j-1] == '*') {
                go = 1;
            }
        }
        else {
            printf("format is not valid\r\n");
        }
    }
    board[i-1][j-1] = X_or_O;
    print_board(board);
    if (pre_grade(board, X_or_O)!=0) {
        /*
        this code is here to solve the double win problem
        before this was here it would only check if you won after the computer played
        now it checks before and after the computer plays.
        */
        com_play(board, X_or_O);
        grade(board, X_or_O);
    }
}

void com_play(char board[][board_size], char X_or_O) {
    char com;
    int i, j;
    int go = 0;
    time_t t;

    srand48((unsigned) time(&t));

    if (X_or_O=='X') {
        com = 'O';
    }
    else if (X_or_O=='O') {
        com = 'X';
    }
    else {
        com = 'O';
    }

    while (go==0) {
        i = (lrand48() % 3);
        j = (lrand48() % 3);
        if (board[i][j]=='X' || board[i][j]=='O') {
            /* Do nothing*/
        }
        else {
            board[i][j] = com;
            go = 1;
        }
    }
    printf("Computers move:\r\n");
    print_board(board);
    return;
}

void grade(char board[][board_size], char X_or_O) {
    char com;

    if (X_or_O=='X') com = 'O';
    else if (X_or_O=='O') com = 'X';
    else com = 'O';

    if (check(board, X_or_O)==0) {
        printf("Player Wins!\r\n");
    }
    else if (check(board, com)==0) {
        printf("Computer Wins\r\n");
    }
    else if (check_tie(board)==0) {
        printf("Tie\r\n");
    }
    else {
        play(board, X_or_O);
    }

}

int pre_grade(char board[][board_size], char X_or_O) {
    char com;

    if (X_or_O=='X') com = 'O';
    else if (X_or_O=='O') com = 'X';
    else com = 'O';

    if (check(board, X_or_O)==0) {
        printf("Player Wins!\r\n");
        return 0;
    }
    else if (check(board, com)==0) {
        printf("Computer Wins\r\n");
        return 0;
    }
    else if (check_tie(board)==0) {
        printf("Tie\r\n");
        return 0;
    }
    else {
        return 1;
    }

}

void print_board(char board[][board_size]) {
    int i, j;

    printf("0\t1\t2\t3\r\n");
    for (i=0;i<3;i++) {
        printf("%i\t",i+1);
        for (j=0;j<3;j++) {
            if (j==2) printf("%c\r\n", board[i][j]);
            else printf("%c\t", board[i][j]);
        }
    }
    return;
}

int check(char board[][board_size], char XO) {
    if (board[0][0] == XO && board[0][1] == XO && board[0][2] == XO) {
        return 0;
    }
    else if (board[1][0]== XO && board[1][1] == XO && board[1][2] == XO) {
        return 0;
    }
    else if (board[2][0]== XO && board[2][1] == XO && board[2][2] == XO) {
        return 0;
    }
    else if (board[0][0]== XO && board[1][1] == XO && board[2][2] == XO) {
        return 0;
    }
    else if (board[0][2]== XO && board[1][1] == XO && board[2][0] == XO) {
        return 0;
    }
    else if (board[0][0]== XO && board[1][0] == XO && board[2][0] == XO) {
        return 0;
    }
    else if (board[0][1]== XO && board[1][1] == XO && board[2][1] == XO) {
        return 0;
    }
    else if (board[0][2]== XO && board[1][2] == XO && board[2][2] == XO) {
        return 0;
    }
    else {
        return 1;
    }
}

int check_tie(char board[][board_size]) {
    int i, j;
    int plays = 0;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (board[i][j] =='X' || board[i][j] == 'O') {
                plays++;
            }
        }
    }

    if (plays==9) {
        return 0;
    }
    else {
        return 1;
    }
}
