#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "raylib.h"
#include "raymath.h"
#include "queue.h"
#include "set.h"
#include "bmpfile.h"

#define BOARD_WIDTH 50
#define BOARD_HEIGHT 50

#define CELL_WIDTH_PX 30

//#define SCREEN_WIDTH 1000
//#define SCREEN_HEIGHT 1000

#define NEIGHBOR_THRESHOLD 3


#define FOV 45.0f


typedef enum {
    paused = 0,
    running = 1
} GameMode;

enum {
    dead = false,
    alive = true
};

GameMode mode = paused;

typedef struct {
    bool alive;
    bool enqueued;
    unsigned int neighbors;
    size_t i;
    size_t j;
} cell;

typedef struct {
    cell cells[BOARD_WIDTH][BOARD_HEIGHT];
    //unsigned int cn_cells[BOARD_WIDTH][BOARD_HEIGHT];
} cell_board;

void setCellBQ(cell_board* board, Queue* q, unsigned int i, unsigned int j, bool status) {
    board->cells[i][j].alive = status;
    if(status) {
        board->cells[i][j].enqueued = true;
        enqueue(q, (uint64_t) &board->cells[i][j]);
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
    for(size_t i = 0; i < BOARD_WIDTH; i++) {
        for(size_t j = 0; j < BOARD_HEIGHT; j++) {
            board->cells[i][j].alive = dead;
            board->cells[i][j].neighbors = 0;
            board->cells[i][j].i = i;
            board->cells[i][j].j = j;
        }
    }
    
    return board;
}

size_t calculateNeighbors(cell_board* board, size_t i_0, size_t j_0) {
    size_t sum = 0;
    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            if(i == 0 && j == 0) {
                continue;
            }

            size_t neighbor_i = (i_0 + i + BOARD_WIDTH) % BOARD_WIDTH;
            size_t neighbor_j = (j_0 + j + BOARD_HEIGHT) % BOARD_HEIGHT;

            //printf("Neighbor at (%u, %u): %u", (unsigned int) used_i, (unsigned int) used_j, (unsigned int) board->cells[used_i][used_j]);
            sum += (unsigned int) board->cells[neighbor_i][neighbor_j].alive;

        }
    }

    return sum;
}

void updateAllNeighbors(cell_board* board) {
    for(int i = 0; i < BOARD_WIDTH; i++)
        for(int j = 0; j < BOARD_HEIGHT; j++)
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

void setEnqueuedNCells(cell_board* board, Queue* q, SimpleSet* s, size_t i_0, size_t j_0) {

    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            //fprintf(stderr, "Getting neighboring cell dims\n");

            size_t neighbor_i = (i_0 + i + BOARD_WIDTH) % BOARD_WIDTH;
            size_t neighbor_j = (j_0 + j + BOARD_HEIGHT) % BOARD_HEIGHT;

            //fprintf(stderr, "Setting neighboring cells\n");
            //printf("Neighbor at (%u, %u): %u", (unsigned int) used_i, (unsigned int) used_j, (unsigned int) board->cells[used_i][used_j]);
            uint64_t key_addr = (uint64_t) ( (neighbor_i << ((unsigned int) log2(BOARD_WIDTH) + 1)) ) + neighbor_j;
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
    //updateAllNeighbors(board);
    resetQueue(q);

    //printf("Calculating neighbors");

    SimpleSet s;
    set_init(&s);

    //printf("Calculating neighbors");
    

    for(size_t i = 0; i < BOARD_WIDTH; i++) {
        for(size_t j = 0; j < BOARD_HEIGHT; j++) {
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
    for(size_t i = 0; i < BOARD_WIDTH; i++) {
        for(size_t j = 0; j < BOARD_HEIGHT; j++) {
            bool cell = board->cells[i][j].alive;
            Color color = BLACK;
            if(cell)
                color = WHITE;

            DrawRectangle(i * CELL_WIDTH_PX / 2, j * CELL_WIDTH_PX / 2, CELL_WIDTH_PX, CELL_WIDTH_PX, color);
        }
    }
    EndDrawing();
}

void printBoard(cell_board* board) {
    fprintf(stderr, "\n### New Board ###\n");
    for(size_t i = 0; i < BOARD_WIDTH; i++) {
        for(size_t j = 0; j < BOARD_HEIGHT; j++) {
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
    for(size_t i = 0; i < BOARD_WIDTH; i++) {
        for(size_t j = 0; j < BOARD_HEIGHT; j++) {
            size_t neighbors = calculateNeighbors(board, i, j);
            fprintf(stderr, "%u ", (unsigned int) neighbors);
        }
        fprintf(stderr, "\n");
    }
}

void randomizeBoard(cell_board* board, Queue* q) {
    const float coverage = 0.2;
    fprintf(stderr, "Randomizing board\n");
    resetQueue(q);

    SimpleSet s;
    set_init(&s);

    fprintf(stderr, "Gotten board set\n");

    for(size_t i = 0; i < BOARD_WIDTH; i++) {
        for(size_t j = 0; j < BOARD_HEIGHT; j++) {
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
    for(size_t i = 0; i < BOARD_WIDTH; i++) {
        for(size_t j = 0; j < BOARD_HEIGHT; j++) {
            board->cells[i][j].alive = dead;
            board->cells[i][j].neighbors = 0;
            //board->cells[i][j].i = i;
            //board->cells[i][j].j = j;
        }
    }
}

void drawTile(cell_board* board, unsigned int mouse_x, unsigned int mouse_y) {
    size_t cell_i = round((float) BOARD_WIDTH * (float) mouse_x / (BOARD_WIDTH * CELL_WIDTH_PX) * 2);
    size_t cell_j = round((float) BOARD_HEIGHT * (float) mouse_y / (BOARD_HEIGHT * CELL_WIDTH_PX) * 2);

    if(cell_i > BOARD_WIDTH || cell_j > BOARD_HEIGHT) {
        return;
    }
    
    //printf("Drawing clicked tile at %zu, %zu ", cell_x, cell_y); 
    //printf("Mouse coords were %u, %u ", mouse_x, mouse_y);
    board->cells[cell_i][cell_j].alive = !board->cells[cell_i][cell_j].alive;
}

void drawClickedTile(cell_board* board) {
    //printf("Drawing clicked tile"); 
    
    drawTile(board, GetMouseX(), GetMouseY());
}

unsigned char** getBoardBytes(cell_board* board) {
    // Allocate memory for the row pointers
    unsigned char** board_gs = malloc(BOARD_WIDTH * sizeof(unsigned char*));
    if (board_gs == NULL) {
        perror("Failed to allocate memory for board_gs");
        return NULL; // Return NULL if allocation fails
    }

    // Allocate memory for each row and set initial values
    for (size_t i = 0; i < BOARD_WIDTH; i++) {
        board_gs[i] = malloc(BOARD_HEIGHT * sizeof(unsigned char));
        if (board_gs[i] == NULL) {
            perror("Failed to allocate memory for board_gs row");
            // Free any previously allocated rows if allocation fails
            for (size_t k = 0; k < i; k++) {
                free(board_gs[k]);
            }
            free(board_gs);
            return NULL;
        }

        for (size_t j = 0; j < BOARD_HEIGHT; j++) {
            board_gs[i][j] = board->cells[i][j].alive ? 255 : 0;
        }
    }

    return board_gs;
}

Image getBoardImageGS(cell_board* board) {
    // TODO: see if i or j need to be swapped here
    unsigned char board_gs[BOARD_WIDTH][BOARD_HEIGHT];

    for(size_t i = 0; i < BOARD_WIDTH; i++) {
        for(size_t j = 0; j < BOARD_WIDTH; j++) {
            if(board->cells[i][j].alive)
                board_gs[i][j] = 255;
            else
                board_gs[i][j] = 0;
        }
    }
    Image img = {
        .data = board_gs,
        .width = BOARD_WIDTH,
        .height = BOARD_HEIGHT,
        .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
        .mipmaps = 1
    };

    return img;
}


int main(int argc, char **argv) {
    srand(time(NULL));

    for (int i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }

    fprintf(stderr, "Started prog\n");

    cell_board* board = init_board(BOARD_WIDTH, BOARD_HEIGHT);
    Queue* queue = malloc(sizeof(Queue));
    initializeQueue(queue);

    if (board == NULL) {
        free(board);
        return 1;  // Return 1 to indicate memory allocation failure
    }
    if (queue == NULL) {
        free(queue);
        return 1;  // Return 1 to indicate memory allocation failure
    }

    fprintf(stderr, "Gotten board\n");
    
    InitWindow(1000, 1000, "GoL (rip Conway) on a torus!");

    Camera camera = { {5.0f, 5.0f, 5.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 2.0f, 0.0f}, FOV, 0.5 };
    //Model model = LoadModelFromMesh(GenMeshTorus(0.25f, 4.0f, BOARD_WIDTH * 2, BOARD_HEIGHT * 2));
    
    
    Mesh torusMesh = GenMeshTorus(0.25f, 4.0f, BOARD_WIDTH, BOARD_HEIGHT);
    //Vector2* texcoords = (Vector2*)torusMesh.texcoords;  // Access UVs

    // Modify UVs here for custom mapping
    //for (int i = 0; i < BOARD_WIDTH * BOARD_HEIGHT; i++) {
    //    texcoords[i].x *= 5.0f;  // Scale UVs as needed
    //    texcoords[i].y *= 5.0f;  // Scale UVs as needed
    //}
    //model.transform = MatrixRotateXYZ((Vector3) {90.0f, 0, 0});
    // Upload modified UVs back to the GPU
    //UpdateMeshBuffer(torusMesh, 1, texcoords, torusMesh.vertexCount * sizeof(Vector2), 0);

    Model model = LoadModelFromMesh(torusMesh);
    // Model drawing position
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    
    SetTargetFPS(120);

    //model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    unsigned int frame_ctr = 0;

    while (!WindowShouldClose()) {
        switch(mode) {
            case paused:
                //DrawText(TextFormat("Paused"), 80, CELL_WIDTH_PX * BOARD_HEIGHT + 20, 20, RED);

                int key_pressed = GetKeyPressed();

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

                if(key_pressed == KEY_S) {
                    fprintf(stderr, "saved board image\n");
                    const unsigned char** b = (const unsigned char**) getBoardBytes(board);
                    
                    
                    bmpfile_t* bmp = bmp_create(BOARD_WIDTH, BOARD_HEIGHT, 24);

                    // Define a red pixel
                    for(size_t i = 0; i < BOARD_WIDTH; i++)
                        for(size_t j = 0; j < BOARD_WIDTH; j++) {
                            rgb_pixel_t pixel = { .red = b[i][j], .green = b[i][j], .blue = b[i][j], .alpha = 0 };
                            // Set the pixel at (50, 50) to red
                            bmp_set_pixel(bmp, i, j, pixel);
                        }

                    // Save the BMP file
                    bmp_save(bmp, "board.bmp");

                    // Clean up
                    bmp_destroy(bmp);

                    free(b);

                }
                
                if(key_pressed == KEY_SPACE) {
                    mode = running;
                    SetWindowTitle("GoL (rip Conway) on a torus! : (running)");
                }
                break;

            case running:
                //DrawText(TextFormat("Running"), 80, CELL_WIDTH_PX * BOARD_HEIGHT + 20, 20, RED);
                if(GetKeyPressed() == KEY_SPACE) {
                    mode = paused;
                    SetWindowTitle("GoL (rip Conway) on a torus! : (paused)");
                }

                break;

            default:
                fprintf(stderr, "Undefined mode! : mode was %u", mode);
                return 1;
                break;
        }

        UpdateCamera(&camera, CAMERA_ORBITAL);
        BeginDrawing();

        if(mode == running && frame_ctr++ == 10) {
            //updateBoard(board, queue);
            //printBoard(board);
            //printf("Updated board");
            
            //Image board_image = getBoardImage(board);

            //UnloadTexture(texture);
            //texture = LoadTextureFromImage(board_image);
            //SetTextureWrap(texture, TEXTURE_WRAP_REPEAT);
            //model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
            //UnloadImage(board_image);

            updateBoard(board, queue);
            printBoard(board);

            Image board_image = getBoardImageGS(board);

            // Unload the previous texture to prevent memory buildup
            //UnloadTexture(texture);

            // Load new texture from the updated image
            Texture2D texture = LoadTextureFromImage(board_image);

            // Set texture wrapping and filtering modes
            SetTextureWrap(texture, TEXTURE_WRAP_REPEAT);
            SetTextureFilter(texture, TEXTURE_FILTER_POINT);

            // Apply the texture to the torus model
            model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

            // Free the image now that it's loaded as a texture
            //UnloadImage(board_image);

            frame_ctr = 0;
        }


        ClearBackground(RAYWHITE);
        BeginMode3D(camera);

        DrawModelEx(model, position, (Vector3) {1.0f, 0.0f, 0.0f}, 90.0f, (Vector3) {1.2f, 1.2f, 1.2f}, WHITE);
        DrawGrid(10, 1.0);

        EndMode3D();


        //DrawRectangle(30, 400, 310, 30, Fade(SKYBLUE, 0.5f));
            //DrawRectangleLines(30, 400, 310, 30, Fade(DARKBLUE, 0.5f));
            //DrawText("MOUSE LEFT BUTTON to CYCLE PROCEDURAL MODELS", 40, 410, 10, BLUE);
        DrawText("Life on a torus :)", 500, 10, 20, DARKBLUE);
        EndDrawing();


    }

    //UnloadTexture(texture); // Unload texture

    // Unload models data (GPU VRAM)
    UnloadModel(model);

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    free(board);
    free(queue);
    return 0;
}