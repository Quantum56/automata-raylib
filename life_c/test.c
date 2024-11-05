#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "raylib.h"

#define BOARD_WIDTH 50
#define BOARD_HEIGHT 50

#define CELL_WIDTH_PX 50

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 1000

#define NEIGHBOR_THRESHOLD 3

//#define SEQUENTIAL

enum {
    dead = 0,
    alive = 1
};

typedef struct {
    bool cells[BOARD_WIDTH][BOARD_HEIGHT];
    unsigned int cn_cells[BOARD_WIDTH][BOARD_HEIGHT];
} cell_board;

cell_board* init_board(size_t width, size_t height) {
    cell_board* board = (cell_board*) malloc(sizeof(cell_board));
    if(board == NULL) {
        return NULL;
    }
    for(size_t i = 0; i < BOARD_WIDTH; i++) {
        for(size_t j = 0; j < BOARD_HEIGHT; j++) {
            board->cells[i][j] = dead;
            board->cn_cells[i][j] = 0;
        }
    }
    
    return board;
}

size_t calculateNeighbors(cell_board* board, size_t pos_i, size_t pos_j) {
    size_t sum = 0;
    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            if(i == 0 && j == 0) {
                continue;
            }

            size_t neighbor_i = (pos_i + i + BOARD_WIDTH) % BOARD_WIDTH;
            size_t neighbor_j = (pos_j + j + BOARD_HEIGHT) % BOARD_HEIGHT;

            //printf("Neighbor at (%u, %u): %u", (unsigned int) used_i, (unsigned int) used_j, (unsigned int) board->cells[used_i][used_j]);
            sum += (unsigned int) board->cells[neighbor_i][neighbor_j];

        }
    }

    return sum;
}

void updateAllNeighbors(cell_board* board) {
    for(int i = 0; i < BOARD_WIDTH; i++)
        for(int j = 0; j < BOARD_HEIGHT; j++)
            board->cn_cells[i][j] = calculateNeighbors(board, i, j);
}

void updateBoard(cell_board* board) {
    updateAllNeighbors(board);

    for(size_t i = 0; i < BOARD_WIDTH; i++) {
        for(size_t j = 0; j < BOARD_HEIGHT; j++) {
            bool cell = board->cells[i][j];
            unsigned int neighbors = board->cn_cells[i][j];
            
            if(cell && (neighbors == NEIGHBOR_THRESHOLD || neighbors == NEIGHBOR_THRESHOLD - 1)) {
                board->cells[i][j] = alive;
            } else if(!cell && neighbors == NEIGHBOR_THRESHOLD) {
                board->cells[i][j] = alive;
            } else {
                board->cells[i][j] = dead;
            }

            //printf("Calculated %u neighbors at (%u, %u)", neighbors, i, j);

        
            
            //DrawRectangle(i*CELL_WIDTH_PX/2, j*CELL_WIDTH_PX/2, CELL_WIDTH_PX, CELL_WIDTH_PX, WHITE);
        }
    }
}

void drawBoard(cell_board* board) {
    BeginDrawing();
    ClearBackground(GRAY);
    for(size_t i = 0; i < BOARD_WIDTH; i++) {
        for(size_t j = 0; j < BOARD_HEIGHT; j++) {
            bool cell = board->cells[i][j];
            Color color = BLACK;
            if(cell)
                color = WHITE;

            DrawRectangle(i*CELL_WIDTH_PX/2, j*CELL_WIDTH_PX/2, CELL_WIDTH_PX, CELL_WIDTH_PX, color);
        }
    }
    EndDrawing();
}

void printBoard(cell_board* board) {
    fprintf(stderr, "\n### New Board ###\n");
    for(size_t i = 0; i < BOARD_WIDTH; i++) {
        for(size_t j = 0; j < BOARD_HEIGHT; j++) {
            bool cell = board->cells[i][j];
            if(cell)
                fprintf(stderr, "*");
            else
                fprintf(stderr, "_");
            fprintf(stderr, " ");
        }
        fprintf(stderr, "\n");
    }
}

void printNeighbors(cell_board* board) {
    fprintf(stderr, "\n### New Board ###\n");
    for(size_t i = 0; i < BOARD_WIDTH; i++) {
        for(size_t j = 0; j < BOARD_HEIGHT; j++) {
            size_t neighbors = calculateNeighbors(board, i, j);
            fprintf(stderr, "%u ", (unsigned int) neighbors);
        }
        fprintf(stderr, "\n");
    }
}

void randomizeBoard(cell_board* board) {
    const float coverage = 0.2;
    for(size_t i = 0; i < BOARD_WIDTH; i++) {
        for(size_t j = 0; j < BOARD_HEIGHT; j++) {
            if((float) rand()/RAND_MAX > (1 - coverage))
                board->cells[i][j] = alive;
            else
                board->cells[i][j] = dead;
        }
    }
}


int main(int argc, char **argv) {
    srand(time(NULL));

    for (int i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }

    fprintf(stderr, "Started prog\n");
    cell_board* board = init_board(BOARD_WIDTH, BOARD_HEIGHT);

    if (board == NULL) {
        return 1;  // Return 1 to indicate memory allocation failure
    }


    fprintf(stderr, "Gotten board\n");
    
    if(BOARD_WIDTH * CELL_WIDTH_PX > SCREEN_WIDTH && BOARD_HEIGHT * CELL_WIDTH_PX > SCREEN_HEIGHT) {
        InitWindow(BOARD_WIDTH * CELL_WIDTH_PX / 2, BOARD_HEIGHT * CELL_WIDTH_PX / 2, "GoL (rip Conway)");
    } else {
        InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GoL (rip Conway)");
    }
    
#ifndef SEQUENTIAL
    SetTargetFPS(30);
#else
    SetTargetFPS(120);
#endif

    randomizeBoard(board);
    printBoard(board);
    
    while (!WindowShouldClose()) {

        if(IsKeyPressed(KEY_SPACE)) {
#ifndef SEQUENTIAL
            //printNeighbors(board);
            randomizeBoard(board);
#else
            updateBoard(board);
#endif
            //printBoard(board);
            
        }

#ifndef SEQUENTIALs
        updateBoard(board);
#endif
        drawBoard(board);
    }
    
    CloseWindow();
    //printf("%zu", sizeof(cell_board));
    free(board);
    return 0;
}