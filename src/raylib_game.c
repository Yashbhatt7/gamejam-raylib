/*******************************************************************************************
*
*   raylib gamejam template
*
*   Code licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022-2026 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>      // Emscripten library
#endif

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // Required for:
#include <string.h>                         // Required for:
#include <math.h>                           // Required for: math functions

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum {
    SCREEN_LOGO = 0,
    SCREEN_TITLE,
    SCREEN_GAMEPLAY,
    SCREEN_ENDING
} GameScreen;

typedef struct {
    Camera2D cam;
    Vector2 smoothed_cam_pos;
    float smooth_cam_speed;
} SmoothCam;

typedef struct {
    int player_health_points;
    float speed;
    Vector2 player_pos;
    Texture2D texture;
    bool player_is_hit;

    // sprite sheet info
    float num_frame;
    float cur_frame;

    Rectangle source;
    Rectangle dest;

    // Direction direction;

    // player body
    Rectangle hitbox;
    Vector2 old_pos;
} Player;

typedef struct {
    int id;
    bool solid;
    Vector2 tile_pos;
    Texture2D texture;
    Rectangle source;
    Rectangle dest;
} Tile;

typedef struct {
    Player *player;
    Tile *tiles;
    SmoothCam *camera;
    Rectangle *tile_rects;
    size_t tile_count;
} GameState;

// TODO: Define your custom data types here
//global state for pausing the game
bool paused = false;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int screenWidth = 720;
static const int screenHeight = 720;

static RenderTexture2D target = { 0 };  // Render texture to render our game
static int frameCounter = 0;

// TODO: Define global variables here, recommended to make them static
// static GameScreen game_screen = 0;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// static void run(void);
static void UpdateDrawFrame(void *arg);      // Update and Draw one frame
static Vector2 Normalize(Vector2 dir);


//--------------------------------------------------------------------------------------------
// Module Functions Definition
//--------------------------------------------------------------------------------------------

// Normalizing vector
Vector2 Normalize(Vector2 dir) {
    float magnitude = sqrt((dir.x * dir.x) + (dir.y * dir.y));

    if (magnitude == 0) {
        return dir;
    }

    dir.x /= magnitude;
    dir.y /= magnitude;

    return dir;
}

void collider_for_x(Player *player, Rectangle hard_rects[], size_t size_of_rects) {
    for (size_t i = 0; i < size_of_rects; ++i) {
        if (CheckCollisionRecs(player->hitbox, hard_rects[i])) {
            player->player_pos.x = player->old_pos.x;
            player->hitbox.x = player->old_pos.x + 24;
            // fmt.println("\n\n                Collided\n\n")
        } else {
            // fmt.println("\n")
        }
    }
}

void collider_for_y(Player *player, Rectangle hard_rects[], size_t size_of_rects) {
    for (size_t i = 0; i < size_of_rects; ++i) {
        if (CheckCollisionRecs(player->hitbox, hard_rects[i])) {
            player->player_pos.y = player->old_pos.y;
            player->hitbox.y = player->old_pos.y + 15;
            // printf("\n\n                Collided\n\n");
        } else {
            // printf("\n");
        }
    }
}

// bool pause_the_game() {
//     if (IsKeyDown(KEY_ENTER)) {
//         return false;
//     }
//     // if (IsKeyDown(KEY_ESCAPE)) {
//     return true;
//     // }
// }

void UpdatePlayer(GameState *gs) {
    Vector2 dir = {0, 0};

    if (IsKeyDown(KEY_S)) {
        dir.y += 1;
        gs->player->source.x = 0;
    }
    if (IsKeyDown(KEY_W)) {
        dir.y -= 1;
        gs->player->source.x = 32 * 1;
    }
    if (IsKeyDown(KEY_D)) {
        dir.x += 1;
        gs->player->source.x = 32 * 2;
    }
    if (IsKeyDown(KEY_A)) {
        dir.x -= 1;
        gs->player->source.x = 32 * 3;
    }

    // fmt.println(dir.x, dir.y)
    printf("%f, %f\n", dir.x, dir.y);
    float dt = GetFrameTime();

    gs->player->old_pos = gs->player->player_pos;
    Vector2 norm_dir;
    norm_dir = Normalize(dir);

    gs->player->player_pos.x += norm_dir.x * gs->player->speed * dt;
    gs->player->hitbox.x = gs->player->player_pos.x + 24;
    collider_for_x(gs->player, gs->tile_rects, gs->tile_count);

    gs->player->player_pos.y += norm_dir.y * gs->player->speed * dt;
    gs->player->hitbox.y = gs->player->player_pos.y + 15;
    collider_for_y(gs->player, gs->tile_rects, gs->tile_count);

    float c = gs->camera->smooth_cam_speed * dt;
    Vector2 target_pos = gs->player->player_pos;
    target_pos.x += 40.0;
    target_pos.y += 50.0;
    gs->camera->cam.target.x += (target_pos.x - gs->camera->cam.target.x) * c;
    gs->camera->cam.target.y += (target_pos.y - gs->camera->cam.target.y) * c;

    gs->player->dest.x = gs->player->player_pos.x;
    gs->player->dest.y = gs->player->player_pos.y;
    printf("\n\n%d\n\n", (int)gs->player->player_pos.x);

}

// Update and draw frame
void UpdateDrawFrame(void *arg) {
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update variables / Implement example logic at this point
    GameState *gs = (GameState*)arg;

    if (IsKeyPressed(KEY_TAB)) {
        paused = !paused;
    }

    if (!paused) {
        UpdatePlayer(gs);
    } else {
        frameCounter++;
    }

    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    // Render game screen to a texture,
    // it could be useful for scaling or further shader postprocessing

    // BeginTextureMode(target);
    BeginDrawing();
    BeginMode2D(gs->camera->cam);

    ClearBackground(RAYWHITE);
        if (paused && (frameCounter/20)%2) {
            // ClearBackground(RAYWHITE);

            // TODO: Draw your game screen here

            DrawRectangle(70, 90, 200, 200, BLACK);
            DrawRectangle(70 + 16, 90 + 16, 200 - 32, 200 - 32, RAYWHITE);
            DrawText("raylib", 70 + 200 - MeasureText("raylib", 40) - 32, 90 + 200 - 40 - 24, 40, BLACK);

            DrawText("6.x", 290, 90 - 26, 280, BLACK);
            DrawText("GAMEJAM", 70, 90 + 210, 120, MAROON);
            DrawText("paused", 160, 500, 50, BLACK);
            DrawRectangleLinesEx((Rectangle){ 0, 0, screenWidth, screenHeight }, 16, BLACK);
            DrawTexturePro(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height },
                    (Rectangle){ 0, 0, (float)target.texture.width, (float)target.texture.height }, (Vector2){ 0, 0 }, 0.0f, WHITE);

        } else {
            DrawTexturePro(gs->player->texture, gs->player->source, gs->player->dest, (Vector2){0, 0}, 0, WHITE);
        }


        EndMode2D();
        EndDrawing();
        // EndTextureMode();

        // Draw render texture to screen, scaled if required

        // TODO: Draw everything that requires to be drawn at this point, maybe UI?



    // if (IsKeyDown(KEY_ENTER)) {
    // }

    // Render to screen (main framebuffer)
    //----------------------------------------------------------------------------------
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------


int main(void) {
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messages
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "GameJam");

    // TODO: Load resources / Initialize variables at this point

    // Render texture to draw, enables screen scaling
    // NOTE: If screen is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);


    GameState gs;

    Rectangle collision_rectangle = { 600, 250, 100, 100 };

    Rectangle arr_of_rects[2] = {
        collision_rectangle,
        // collision_rectangle1,
        // collision_rectangle2,
    };

    size_t size_of_rects = sizeof(arr_of_rects) / sizeof(arr_of_rects[0]);


    Player player = {
        .player_health_points = 5,
        .speed = 300,
        .texture = LoadTexture("./resources/assets/player/sqPlayer2.png"),
        .player_is_hit = false,

        .num_frame = 4,
        .cur_frame = 0,
        // source refers to -> from where to start drawing
        .source = {
            .x = 0.0,
            .y = 0.0,
            .width = 32.0,
            .height = 32.0,
        },

        .dest = {
            .x = 30.0,
            .y = 30.0,
            .width = 32.0 * 3,
            .height = 32.0 * 3,
        },

        .hitbox = {
            .width = 32.0 * 3 - 48,
            .height = 32.0 * 3 - 15,
        },
    };


    Tile tiles = {
        .texture = LoadTexture("./resources/assets/tiles/ground.png"),

        .source = {
            .x = 0,
            .y = 0,
            .width = 16,
            .height = 16,
        },

        .dest = {
            .x = 0,
            .y = 0,
            .width = 16 * 5,
            .height = 16 * 5,
        },
    };

    tiles.tile_pos.x = 600;
    tiles.tile_pos.y = 300;

    SmoothCam camera = {
        .smooth_cam_speed = 6.0,
        .smoothed_cam_pos = {0, 0},

        .cam = {
            .offset = { screenWidth / 2.0, screenHeight / 2.0 },
            // target = player.player_pos,
            .rotation = 0.0,
            .zoom = 1.3,
        }
    };


#if defined(PLATFORM_WEB)
    gs.tile_rects = arr_of_rects;
    gs.tile_count = size_of_rects;
    gs.player = &player;
    gs.tiles = &tiles;
    gs.camera = &camera;
    emscripten_set_main_loop_arg(UpdateDrawFrame, &gs, 60, 1);
#else
    gs.tile_rects = arr_of_rects;
    gs.tile_count = size_of_rects;
    gs.player = &player;
    gs.tiles = &tiles;
    gs.camera = &camera;
    SetTargetFPS(60);     // Set our game frames-per-second
                          //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) {    // Detect window close button
        UpdateDrawFrame(&gs);
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);

    // TODO: Unload all loaded resources at this point

    CloseWindow();        // Close window and OpenGL context
                          //--------------------------------------------------------------------------------------

    return 0;
}

