#include "main.h"

#define SDC_IMPLEMENTATION
#include "sdc.h"

#define BOARD_SZ 3

typedef enum {
    NONE = 0,
    PLAYER = 1,
    BOT = 2,
} Party;

typedef enum {
    WIN_NONE,
    WIN_PLAYER,
    WIN_BOT,
    WIN_DRAW,
} WinState;

typedef struct {
    Party is;
} BoardCell;

global BoardCell board[BOARD_SZ][BOARD_SZ];
Party current_turn;
WinState win_state = WIN_NONE;

void Board_init(void);
void Board_print(void);

int Player_move(void);
int Bot_move(void) { return SDC_rand_range(1, BOARD_SZ * BOARD_SZ + 1); }
void execute_move(int move, Party t);

void check_win_state(void);
char *end_message(void);

int main(void) {
    Board_init();

    while (win_state == WIN_NONE) {
        while (win_state == WIN_NONE) {
            Board_print();
            execute_move(Player_move(), PLAYER);
            check_win_state();
            if (current_turn != NONE)
                break;
        }
        while (win_state == WIN_NONE) {
            execute_move(Bot_move(), BOT);
            check_win_state();
            if (current_turn != NONE)
                break;
        }
    }

    Board_print();
    puts(end_message());

    return 0;
}

void Board_init(void) {
    int i, j;
    for (i = 0; i < BOARD_SZ; i++) {
        for (j = 0; j < BOARD_SZ; j++) {
            board[i][j] = (BoardCell){.is = NONE};
        }
    }
}

int Player_move(void) {
    int i, j, k = 1;
    char *input_buffer;

    printf("Available moves:\n");
    for (i = 0; i < BOARD_SZ; i++) {
        for (j = 0; j < BOARD_SZ; j++) {
            if (board[i][j].is == NONE)
                printf("%-3d", k);
            else
                printf("%-3s", ".");
            k++;
        }
        printf("\n");
    }
    SDC_io_prompt(&input_buffer, "Pick your next move!");
    return atoi(input_buffer);
}

void execute_move(int move, Party p) {
    int i, j, k = 1;
    for (i = 0; i < BOARD_SZ; i++) {
        for (j = 0; j < BOARD_SZ; j++) {
            if (board[i][j].is == NONE && k == move) {
                board[i][j].is = p;
                current_turn = PLAYER;
                return;
            }
            k++;
        }
    }
    if (p == PLAYER)
        printf("That move is not legal! Try again...\n");
    current_turn = NONE;
}

void Board_print(void) {
    int i, j;
    printf("Current board:\n");
    for (i = 0; i < BOARD_SZ; i++) {
        for (j = 0; j < BOARD_SZ; j++) {
            switch (board[i][j].is) {
                case PLAYER:
                    printf("%-3s", "x");
                    break;
                case BOT:
                    printf("%-3s", "o");
                    break;
                default:
                    printf("%-3s", "`");
                    break;
            }
        }
        printf("\n");
    }
}

char *end_message(void) {
    switch (win_state) {
        case WIN_PLAYER:
            return "-----\nYou won!";
        case WIN_BOT:
            return "-----\nYou lost against a dumb robot!";
        default:
            return "-----\nEveryone wins!";
    }
}

global const int board_state_win_conditions[8][BOARD_SZ][BOARD_SZ] = {
    {{1, 1, 1}, {0, 0, 0}, {0, 0, 0}}, {{0, 0, 0}, {1, 1, 1}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {1, 1, 1}}, {{1, 0, 0}, {1, 0, 0}, {1, 0, 0}},
    {{0, 1, 0}, {0, 1, 0}, {0, 1, 0}}, {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}},
    {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}, {{0, 0, 1}, {0, 1, 0}, {1, 0, 0}},
};

void check_win_state(void) {
    int i, j, win_cond;
    int player_matches;
    int bot_matches;
    int board_full = 1;

    for (win_cond = 0; win_cond < 8; win_cond++) {
        player_matches = 0;
        bot_matches = 0;

        for (i = 0; i < BOARD_SZ; i++) {
            for (j = 0; j < BOARD_SZ; j++) {

                if (board[i][j].is == NONE) {
                    board_full = 0;
                }

                if (board_state_win_conditions[win_cond][i][j] == 1) {
                    if (board[i][j].is == PLAYER) {
                        player_matches++;
                    } else if (board[i][j].is == BOT) {
                        bot_matches++;
                    }
                }
            }
        }

        if (player_matches == BOARD_SZ) {
            win_state = WIN_PLAYER;
            return;
        }

        if (bot_matches == BOARD_SZ) {
            win_state = WIN_BOT;
            return;
        }
    }

    if (board_full) {
        win_state = WIN_DRAW;
    } else {
        win_state = WIN_NONE;
    }
}
