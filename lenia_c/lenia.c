#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <assert.h>

//#define OMP

#ifdef OMP
#include <omp.h>
#endif


#ifdef _WIN32
  #include <windows.h>
  #include <synchapi.h>
#elif __linux__
  #include <unistd.h>
  #include <sys/types.h>
#elif __APPLE__
  #include <TargetConditionals.h>
  #if TARGET_OS_MAC
    #include <unistd.h>
  #endif
#endif

#include "raylib.h"
#include "queue_d.h"
#include "set.h"

#define WAIT


//#define CELL_WIDTH_PX 30

//#define SCREEN_WIDTH 1000
//#define SCREEN_HEIGHT 1000

#define NEIGHBOR_THRESHOLD 3

#define COVERAGE 0.2f

#define GLIDER_I_0 12
#define GLIDER_J_0 4

typedef enum {
    paused = 0,
    running = 1
} GameMode;

enum {
    dead = false,
    alive = true
};

GameMode mode = paused;

// TODO: dynamically allocate cells depending on size specified at runtime
size_t board_width = 50; // default
size_t board_height = 50; // default
unsigned int cell_width_px = 1; // default

typedef struct {
    bool alive;
    bool enqueued;
    unsigned short neighbors;
    size_t i;
    size_t j;
} cell;

typedef struct {
    cell** cells;
    size_t board_width;
    size_t board_height;
    // TODO: dynamically allocate cells depending on size specified at runtime

    //unsigned int cn_cells[BOARD_WIDTH][BOARD_HEIGHT];
} cell_board;

void setEnqueuedNCells(cell_board* board, Queue* q, SimpleSet* s, size_t i_0, size_t j_0);

void setCellBQ(cell_board* board, Queue* q, unsigned int i, unsigned int j, bool status) {
    board->cells[i][j].alive = status;
    if(status) {
        board->cells[i][j].enqueued = true;
        setEnqueuedNCells(board, q, NULL, i, j);
    }
}

void setCellB(cell_board* board, unsigned int i, unsigned int j, bool status) {
    board->cells[i][j].alive = status;
}

void setCellC(cell* c, bool status) {
    c->alive = status;
}


cell_board* init_board(size_t width, size_t height) {
    cell_board* board = (cell_board*) malloc(sizeof(cell_board));

    if(board == NULL) {
        return NULL;
    }

    board->cells = malloc(width * sizeof(cell*));
    if(board->cells == NULL) {
        free(board);
        return NULL;
    }

    for (size_t i = 0; i < width; i++)
        board->cells[i] = malloc(height * sizeof(cell));

    board->board_width = width;
    board->board_height = height;

    if(board->cells == NULL) {
        free(board);
        return NULL;
    }

    for(size_t i = 0; i < width; i++) {
        for(size_t j = 0; j < height; j++) {
            board->cells[i][j].alive = dead;
            board->cells[i][j].neighbors = 0;
            board->cells[i][j].i = i;
            board->cells[i][j].j = j;
        }
    }
    
    return board;
}

void free_board(cell_board* board) {
    if(board && board->cells) {
        for (size_t i = 0; i < board->board_width; i++)
            free(board->cells[i]);  // Free each row
        free(board->cells);
    }
    if(board) {
        free(board);
    }
}


size_t calculateNeighbors(cell_board* board, size_t i_0, size_t j_0) {
    size_t sum = 0;
    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            if(i == 0 && j == 0) {
                continue;
            }
            
            #ifdef OMP
            #pragma omp critical
            #endif
            {
            size_t neighbor_i = (i_0 + i + board_width) % board_width;
            size_t neighbor_j = (j_0 + j + board_height) % board_height;
            
            //printf("Neighbor at (%u, %u): %u", (unsigned int) used_i, (unsigned int) used_j, (unsigned int) board->cells[used_i][used_j]);
            sum += (unsigned int) board->cells[neighbor_i][neighbor_j].alive;
            }

        }
    }

    return sum;
}

void updateAllNeighbors(cell_board* board) {
    
    for(size_t i = 0; i < board_width; i++)
        #pragma omp parallel for
        for(size_t j = 0; j < board_height; j++)
            board->cells[i][j].neighbors = calculateNeighbors(board, i, j);
}

void updateEnqueuedNeighbors(cell_board* board, Queue* q) {
    //printf("Getting enqueued neighbors");
    while(!isEmpty(q)) {
        cell* c = (cell*) peek(q);

        if(c) {
            c->neighbors = calculateNeighbors(board, c->i, c->j);
        }

        dequeue(q);
    }

    //free();

}

void updateEnqueuedNeighborsFor(cell_board* board, Queue* q) {
    //printf("Getting enqueued neighbors");
    cell** active_cells = calloc(q->size, sizeof(cell*));

    size_t ctr = 0;
    while(!isEmpty(q)) {
        cell* c = (cell*) peek(q);
        if(c)
            active_cells[ctr++] = c;

        dequeue(q);
    }

    assert(ctr != 0 && isEmpty(q));
    
    #ifdef OMP
    #pragma omp parallel for
    #endif
    for(size_t i = 0; i < ctr; i++) {
        cell* c = active_cells[i];
        if(c) {
            c->neighbors = calculateNeighbors(board, c->i, c->j);
        }
    }

    free(active_cells);

}

void setEnqueuedNCells(cell_board* board, Queue* q, SimpleSet* s, size_t i_0, size_t j_0) {

    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            //fprintf(stderr, "Getting neighboring cell dims\n");

            size_t neighbor_i = (i_0 + i + board_width) % board_width;
            size_t neighbor_j = (j_0 + j + board_height) % board_height;

            //fprintf(stderr, "Setting neighboring cells\n");
            //printf("Neighbor at (%u, %u): %u", (unsigned int) used_i, (unsigned int) used_j, (unsigned int) board->cells[used_i][used_j]);
            uint64_t key_addr = (uint64_t) ( (neighbor_i << ((unsigned int) log2(board_width) + 1)) ) + neighbor_j;
            char key[8];

            for (int i = 0; i < 8; i++)
                key[i] = (key_addr >> (i * 8)) & 0xFF;
            
            //printf(key);
            
            if(!s) {
                board->cells[neighbor_i][neighbor_j].enqueued = true;
                enqueue(q, (uint64_t) &board->cells[neighbor_i][neighbor_j]);
            }
            else if(set_add(s, (const char*) key) == SET_TRUE) {
                board->cells[neighbor_i][neighbor_j].enqueued = true;
                enqueue(q, (uint64_t) &board->cells[neighbor_i][neighbor_j]);
            }
            else
                board->cells[neighbor_i][neighbor_j].enqueued = false;
        }
    }
    
}

void updateBoard(cell_board* board, Queue* q) {
    updateEnqueuedNeighbors(board, q);
    //updateEnqueuedNeighborsFor(board, q);
    //updateAllNeighbors(board);
    resetQueue(q);

    //printf("Calculating neighbors");

    SimpleSet s;
    set_init(&s);

    //printf("Calculating neighbors");
    

    for(size_t i = 0; i < board_width; i++) {
        #ifdef OMP
        #pragma omp parallel for
        #endif
        for(size_t j = 0; j < board_height; j++) {
            bool cell = board->cells[i][j].alive;
            unsigned int neighbors = board->cells[i][j].neighbors;
            
            if(cell && (neighbors == NEIGHBOR_THRESHOLD || neighbors == NEIGHBOR_THRESHOLD - 1)) {
                setCellB(board, i, j, alive);
                
                setEnqueuedNCells(board, q, &s, i, j);
            } else if(!cell && neighbors == NEIGHBOR_THRESHOLD) {
                setCellB(board, i, j, alive);

                setEnqueuedNCells(board, q, &s, i, j);
            } else {
                setCellB(board, i, j, dead);
            }
        }
    }

    set_destroy(&s);
}

void drawBoard(cell_board* board) {
    BeginDrawing();
    ClearBackground(GRAY);
    for(size_t i = 0; i < board_width; i++) {
        for(size_t j = 0; j < board_height; j++) {
            bool cell = board->cells[i][j].alive;
            Color color = BLACK;
            if(cell)
                color = WHITE;

            DrawRectangle(i * cell_width_px / 2, j * cell_width_px / 2, cell_width_px, cell_width_px, color);
        }
    }
    EndDrawing();
}

void printBoard(cell_board* board) {
    fprintf(stderr, "\n### New Board ###\n");
    for(size_t i = 0; i < board_width; i++) {
        for(size_t j = 0; j < board_height; j++) {
            bool cell = board->cells[i][j].alive;
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
    for(size_t i = 0; i < board_width; i++) {
        for(size_t j = 0; j < board_height; j++) {
            size_t neighbors = calculateNeighbors(board, i, j);
            fprintf(stderr, "%u ", (unsigned int) neighbors);
        }
        fprintf(stderr, "\n");
    }
}

void randomizeBoard(cell_board* board, Queue* q) {
    const float coverage = COVERAGE;
    fprintf(stderr, "Randomizing board\n");
    resetQueue(q);

    SimpleSet s;
    set_init(&s);

    fprintf(stderr, "Gotten board set\n");

    for(size_t i = 0; i < board_width; i++) {
        for(size_t j = 0; j < board_height; j++) {
            if((float) rand()/RAND_MAX > (1 - coverage)) {
                //fprintf(stderr, "Setting single cell\n");
                setCellB(board, i, j, alive);
                //fprintf(stderr, "Setting neighboring cells to queue\n");
                setEnqueuedNCells(board, q, &s, i, j);
            }
            else
                setCellB(board, i, j, dead);
        }
    }
    //printQueue(q);
    set_destroy(&s);
}


void clearBoard(cell_board* board) {
    for(size_t i = 0; i < board_width; i++) {
        for(size_t j = 0; j < board_height; j++) {
            board->cells[i][j].alive = dead;
            board->cells[i][j].neighbors = 0;
            //board->cells[i][j].i = i;
            //board->cells[i][j].j = j;
        }
    }
}

void drawTile(cell_board* board, Queue* q, unsigned int mouse_x, unsigned int mouse_y) {
    size_t cell_i = round((float) board_width * (float) mouse_x / (board_width * cell_width_px) * 2);
    size_t cell_j = round((float) board_height * (float) mouse_y / (board_height * cell_width_px) * 2);

    if(cell_i > board_width || cell_j > board_height) {
        return;
    }
    
    //printf("Drawing clicked tile at %zu, %zu ", cell_x, cell_y); 
    //printf("Mouse coords were %u, %u ", mouse_x, mouse_y);
    bool* status = &board->cells[cell_i][cell_j].alive;

    *status = !*status;

    if(board->cells[cell_i][cell_j].alive)
        setEnqueuedNCells(board, q, NULL, cell_i, cell_j);
}

void drawClickedTile(cell_board* board, Queue* q) {
    //printf("Drawing clicked tile"); 
    
    drawTile(board, q, GetMouseX(), GetMouseY());
}


void setBoardGlider(cell_board* board, Queue* q, size_t i_0, size_t j_0) {
    bool glider[3][3] = { {dead, alive, dead}, {dead, dead, alive}, {alive, alive, alive} };

    for(size_t i = 0; i < 3; i++) {
        for(size_t j = 0; j < 3; j++) {
            setCellBQ(board, q, j + j_0, i + i_0, glider[i][j]);
        }
    }

}

unsigned long safe_atoi(const char *str) {
    unsigned long value;
    if (sscanf(str, "%zu", &value) == 1) {
        return value;
    } else {
        // Handle conversion error
        fprintf(stderr, "Invalid input\n");
        return 0;
    }
}

int main(int argc, char** argv) {
    srand(time(NULL));


    //fprintf(stderr, "Num args: %i", argc);
    if(argc >= 3) {
        for (int i = 0; i < argc; i++) {
            printf("%s\n", argv[i]);
        }

        board_width = (size_t) safe_atoi(argv[1]);
        board_height = (size_t) safe_atoi(argv[2]);

        if(argc >= 4) {
            cell_width_px = (unsigned) safe_atoi(argv[3]);
        }
    }



    fprintf(stderr, "Started prog, board = (%zu x %zu)\n", board_width, board_height);

    cell_board* board = init_board(board_width, board_height);
    
    Queue* queue = malloc(sizeof(Queue));
    initializeQueue(queue, board->board_width * board->board_height);

    if (board == NULL) {
        return 1;  // Return 1 to indicate memory allocation failure
    }
    if (queue == NULL) {
        return 1;  // Return 1 to indicate memory allocation failure
    }

    fprintf(stderr, "Gotten board\n");
    
    InitWindow(board_width * cell_width_px / 2, board_height * cell_width_px / 2, "GoL (rip Conway)");

    SetTargetFPS(120);

    //randomizeBoard(board);
    //printBoard(board);
    
    while (!WindowShouldClose()) {
        
        int key_pressed = GetKeyPressed();

        if(key_pressed == KEY_KP_EQUAL) {
            // TODO: resize screen
        }

        if(key_pressed == KEY_KP_SUBTRACT) {
            // TODO: resize screen
        }

        switch(mode) {

            case paused:
                //DrawText(TextFormat("Paused"), 80, CELL_WIDTH_PX * BOARD_HEIGHT + 20, 20, RED);

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    drawClickedTile(board, queue);
                }

                if(key_pressed == KEY_R) {
                    clearBoard(board);
                    randomizeBoard(board, queue);
                }

                if(key_pressed == KEY_C) {
                    clearBoard(board);
                    fprintf(stderr, "cleared board\n");
                    
                    resetQueue(queue);
                    fprintf(stderr, "reset queue\n");
                }

                if(key_pressed == KEY_G) {
                    clearBoard(board);
                    fprintf(stderr, "cleared board\n");
                    
                    resetQueue(queue);
                    fprintf(stderr, "reset queue\n");

                    setBoardGlider(board, queue, GLIDER_I_0, GLIDER_J_0);
                }
                
                if(key_pressed == KEY_SPACE) {
                    mode = running;
                    SetTargetFPS(30);
                    SetWindowTitle("GoL (rip Conway) : (running)");
                    continue;
                }

                break;

            case running:
                //DrawText(TextFormat("Running"), 80, CELL_WIDTH_PX * BOARD_HEIGHT + 20, 20, RED);
#ifdef WAIT

#ifdef __linux__
                usleep((__useconds_t) 100);
#elif _WIN32
                Sleep((DWORD) 1);
#elif __APPLE__
                usleep((useconds_t) 100);
#endif

#endif
                if(key_pressed == KEY_SPACE) {
                    mode = paused;
                    SetTargetFPS(120);
                    SetWindowTitle("GoL (rip Conway) : (paused)");
                    continue;
                }

                if(key_pressed == KEY_EQUAL) {

                }

                if(key_pressed == KEY_MINUS) {
                    
                }

                updateBoard(board, queue);

                break;

            default:
                fprintf(stderr, "Undefined mode! : mode was %u", mode);
                return 1;
                break;
        }

        //updateBoard(board);
        drawBoard(board);
    }
    
    CloseWindow();
    //printf("%zu", sizeof(cell_board));
    free_board(board);
    freeQueue(queue);
    return 0;
}