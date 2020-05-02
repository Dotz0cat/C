#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>

void play(char board[][3], char X_or_O, int inner);
void com_play(char board[][3], char X_or_O, int inner);
void grade(char board[][3], char X_or_O, int inner);
void print_board(char board[][3], int inner);
int check(char board[][3], char XO, int inner);


int main()
{
    char X_or_O;
    int go = 0;
    char board[3][3] = {
        {'*','*','*'},
        {'*','*','*'},
        {'*','*','*'}
    };
    while (go==0) {
        printf("Xs or Os?\r\n");
        scanf("%c", &X_or_O);
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
    play(&board, X_or_O, 3);
    return 0;
}

void play(char board[][3], char X_or_O, int inner) {
    int i, j, play_valid;
    int go = 0;
    char place_to_play[6];

    printf("Player's move:\r\n");
    print_board(board, 3);

    while (go==0) {
        printf("Where do you want to play?\r\nFormat(x,y)\r\n");
        scanf("%s", &place_to_play);
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
            go = 1;
        }
        else {
            printf("format is not valid\r\n");
        }
    }
    board[i-1][j-1] = X_or_O;
    print_board(board, 3);
    com_play(&board, X_or_O, 3);
    grade(&board, X_or_O, 3);
}

void com_play(char board[][3], char X_or_O, int inner) {
    char com;
    int i, j;

    if (X_or_O=='X') {
        com = 'O';
    }
    else if (X_or_O=='O') {
        com = 'X';
    }

    i = (rand() % (3-1+1)+1);
    j = (rand() % (3-1+1)+1);
    board[i][j] = com;
    printf("Computers move:\r\n");
    print_board(board, 3);
    return;
}

void grade(char board[][3], char X_or_O, int inner) {
    char com;

    if (X_or_O=='X') com = 'O';
    else if (X_or_O=='O') com = 'X';

    if (check(board, X_or_O, 3)==0) {
        printf("Player Wins!");
    }
    else if (check(board, com, 3)==0) {
        printf("Computer Wins");
    }
    else {
        play(&board, X_or_O, 3);
    }

}

void print_board(char board[][3], int inner) {
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

int check(char board[][3], char XO, int inner) {
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
