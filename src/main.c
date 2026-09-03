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

global BoardCell main_board[BOARD_SZ][BOARD_SZ];
Party current_turn;
WinState win_state = WIN_NONE;

void Board_init(void);
void Board_print(BoardCell b[BOARD_SZ][BOARD_SZ]);

int Player_move(void);
int Bot_move(void);
void execute_move(int move, Party p, BoardCell b[BOARD_SZ][BOARD_SZ]);

WinState check_win_state(BoardCell b[BOARD_SZ][BOARD_SZ]);

char *end_message(void);

int main(void) {
    Board_init();

    while (win_state == WIN_NONE) {
        while (win_state == WIN_NONE) {
            Board_print(main_board);
            execute_move(Player_move(), PLAYER, main_board);
            win_state = check_win_state(main_board);
            if (current_turn == NONE)
                printf("That move is not legal! Try again...\n");
            if (current_turn != NONE)
                break;
        }
        while (win_state == WIN_NONE) {
            execute_move(Bot_move(), BOT, main_board);
            win_state = check_win_state(main_board);
            if (current_turn != NONE)
                break;
        }
    }

    Board_print(main_board);
    puts(end_message());

    return 0;
}

void Board_init(void) {
    int i, j;
    for (i = 0; i < BOARD_SZ; i++) {
        for (j = 0; j < BOARD_SZ; j++) {
            main_board[i][j] = (BoardCell){.is = NONE};
        }
    }
}

void Board_copy(BoardCell board_copy[BOARD_SZ][BOARD_SZ]) {
    int i, j;
    for (i = 0; i < BOARD_SZ; i++) {
        for (j = 0; j < BOARD_SZ; j++) {
            board_copy[i][j] = main_board[i][j];
        }
    }
}

int Bot_move(void) {
    const int max_att = BOARD_SZ * BOARD_SZ;
    int att_count, r;

    // if the center cell is unoccupied, always take it
    {
        BoardCell board_copy[BOARD_SZ][BOARD_SZ];
        Board_copy(board_copy);
        if (board_copy[1][1].is == NONE)
            return 5;
    }

    // bot looks one move ahead to see if the players next move could make the
    // bot lose or the bots next move could make the bot win
    for (att_count = max_att; att_count > 0; att_count--) {
        BoardCell board_copy[BOARD_SZ][BOARD_SZ];
        Board_copy(board_copy);
        r = SDC_rand_range(1, BOARD_SZ * BOARD_SZ + 1);
        execute_move(r, PLAYER, board_copy);
        if (check_win_state(board_copy) == WIN_PLAYER ||
                check_win_state(board_copy) == WIN_BOT)
            return r;
    }
    return SDC_rand_range(1, BOARD_SZ * BOARD_SZ + 1);
}

int Player_move(void) {
    char *input_buffer;
    SDC_io_prompt(&input_buffer, "Pick your next move!");
    return atoi(input_buffer);
}

void execute_move(int move, Party p, BoardCell b[BOARD_SZ][BOARD_SZ]) {
    int i, j, k = 1;
    for (i = 0; i < BOARD_SZ; i++) {
        for (j = 0; j < BOARD_SZ; j++) {
            if (b[i][j].is == NONE && k == move) {
                b[i][j].is = p;
                current_turn = PLAYER;
                return;
            }
            k++;
        }
    }
    current_turn = NONE;
}

void Board_print(BoardCell b[BOARD_SZ][BOARD_SZ]) {
    int i, j, k = 1;
    printf("Current board:\n");
    for (i = 0; i < BOARD_SZ; i++) {
        for (j = 0; j < BOARD_SZ; j++) {
            switch (b[i][j].is) {
                case PLAYER:
                    printf("%-3s", "x");
                    break;
                case BOT:
                    printf("%-3s", "o");
                    break;
                default:
                    printf("\033[38;5;236m%-3d\033[0m", k);

                    break;
            }
            k++;
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

WinState check_win_state(BoardCell b[BOARD_SZ][BOARD_SZ]) {
    int i, j, win_cond;
    int player_matches;
    int bot_matches;
    int board_full = 1;

    for (win_cond = 0; win_cond < 8; win_cond++) {
        player_matches = 0;
        bot_matches = 0;

        for (i = 0; i < BOARD_SZ; i++) {
            for (j = 0; j < BOARD_SZ; j++) {

                if (b[i][j].is == NONE)
                    board_full = 0;

                if (board_state_win_conditions[win_cond][i][j] == 1) {
                    if (b[i][j].is == PLAYER)
                        player_matches++;
                    else if (b[i][j].is == BOT)
                        bot_matches++;
                }
            }
        }

        if (player_matches == BOARD_SZ)
            return WIN_PLAYER;

        if (bot_matches == BOARD_SZ)
            return WIN_BOT;
    }

    if (board_full)
        return WIN_DRAW;
    else
        return WIN_NONE;
}
