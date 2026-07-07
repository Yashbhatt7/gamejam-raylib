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

// typedef struct {
//     Texture2D texture;
//     Vector2 position;
// } Button;

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
// static GameState gs = { 0 };
static Player player = { 0 };
static Tile tiles = { 0 };
static SmoothCam camera = { 0 };
static Rectangle arr_of_rects[2];
static size_t size_of_rects = sizeof(arr_of_rects) / sizeof(arr_of_rects[0]);

static bool first_time = true;
static bool paused = false;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int screenWidth = 720;
static const int screenHeight = 720;
Texture2D title_screen;
Texture2D pause_screen;

static RenderTexture2D target = { 0 };  // Render texture to render our game
static int frameCounter = 0;

// TODO: Define global variables here, recommended to make them static
// static GameScreen game_screen = 0;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// static void run(void);
static void UpdateDrawFrame(void);      // Update and Draw one frame
static void InitGame();
static Vector2 Normalize(Vector2 dir);
static void collider_for_x(Player *player, Rectangle hard_rects[], size_t size_of_rects);
static void collider_for_y(Player *player, Rectangle hard_rects[], size_t size_of_rects);

//--------------------------------------------------------------------------------------------
// Module Functions Definition
//--------------------------------------------------------------------------------------------

void InitGame() {
    Rectangle collision_rectangle = { 600, 250, 100, 100 };
    Rectangle collision_rectangle1 = { 200, 600, 100, 100 };

    arr_of_rects[0] = collision_rectangle;
    arr_of_rects[0] = collision_rectangle1;

    // Initialize player
    player.player_health_points = 5,
    player.speed = 300,
    player.texture = LoadTexture("./resources/assets/player/sqPlayer2.png"),
    player.player_is_hit = false,

    player.num_frame = 4,
    player.cur_frame = 0,
    // source refers to -> from where to start drawing
    player.source.x = 0.0;
        player.source.y = 0.0;
        player.source.width = 32.0;
        player.source.height = 32.0;

    player.dest.x = 30.0;
        player.dest.y = 30.0;
        player.dest.width = 32.0 * 3;
        player.dest.height = 32.0 * 3;

    player.hitbox.width = 32.0 * 3 - 48;
        player.hitbox.height = 32.0 * 3 - 15;


    tiles.texture = LoadTexture("./resources/assets/tiles/ground.png"),

    tiles.source.x = 0;
        tiles.source.y = 0;
        tiles.source.width = 16;
        tiles.source.height = 16;

    tiles.dest.x = 0;
        tiles.dest.y = 0;
        tiles.dest.width = 16 * 5;
        tiles.dest.height = 16 * 5;

    tiles.tile_pos.x = 600;
    tiles.tile_pos.y = 300;

    camera.smooth_cam_speed = 6.0;
    camera.smoothed_cam_pos = (Vector2){0, 0};

    camera.cam.offset = (Vector2){ screenWidth / 2.0, screenHeight / 2.0 };
        // camera.cam.target = player.player_pos;
        camera.cam.rotation = 0.0;
        camera.cam.zoom = 1.0;
}

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

void UpdatePlayer() {
    Vector2 dir = {0, 0};

    if (IsKeyDown(KEY_S)) {
        dir.y += 1;
        player.source.x = 0;
    }
    if (IsKeyDown(KEY_W)) {
        dir.y -= 1;
        player.source.x = 32 * 1;
    }
    if (IsKeyDown(KEY_D)) {
        dir.x += 1;
        player.source.x = 32 * 2;
    }
    if (IsKeyDown(KEY_A)) {
        dir.x -= 1;
        player.source.x = 32 * 3;
    }

    // fmt.println(dir.x, dir.y)
    printf("%f, %f\n", dir.x, dir.y);
    float dt = GetFrameTime();

    player.old_pos = player.player_pos;
    Vector2 norm_dir;
    norm_dir = Normalize(dir);

    player.player_pos.x += norm_dir.x * player.speed * dt;
    player.hitbox.x = player.player_pos.x + 24;
    collider_for_x(&player, arr_of_rects, size_of_rects);

    player.player_pos.y += norm_dir.y * player.speed * dt;
    player.hitbox.y = player.player_pos.y + 15;
    collider_for_y(&player, arr_of_rects, size_of_rects);

    float c = camera.smooth_cam_speed * dt;
    Vector2 target_pos = player.player_pos;
    target_pos.x += 40.0;
    target_pos.y += 50.0;
    camera.cam.target.x += (target_pos.x - camera.cam.target.x) * c;
    camera.cam.target.y += (target_pos.y - camera.cam.target.y) * c;

    player.dest.x = player.player_pos.x;
    player.dest.y = player.player_pos.y;
    printf("\n\n%d\n\n", (int)player.player_pos.x);
}

// Update and draw frame
void UpdateDrawFrame(void) {
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update variables / Implement example logic at this point
    if (IsKeyPressed(KEY_TAB)) {
        paused = !paused;
        if (paused && IsKeyDown(KEY_SPACE)) {
            paused = false;
        }
    }

    if (!paused) {
        UpdatePlayer();
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
    if (paused && (frameCounter/20)%2) {
    //     DrawText("paused", 160, 500, 50, BLACK);
    }
    BeginMode2D(camera.cam);

    ClearBackground(RAYWHITE);
    if (first_time) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        // DrawTexture(title_screen,  screenWidth/2, screenHeight/2, WHITE);
        // DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint);
        DrawTextureEx(title_screen, (Vector2){(float)screenWidth/2 - 320, (float)screenHeight/2 - 340}, 0, 5, WHITE);
        if (IsKeyDown(KEY_SPACE)) {
            first_time = false;
        }
    } else if (!first_time) {
        if (paused) {
            // ClearBackground(RAYWHITE);

            // TODO: Draw your game screen here

            // DrawRectangle(70, 90, 200, 200, BLACK);
            // DrawRectangle(70 + 16, 90 + 16, 200 - 32, 200 - 32, RAYWHITE);
            // DrawText("raylib", 70 + 200 - MeasureText("raylib", 40) - 32, 90 + 200 - 40 - 24, 40, BLACK);

            DrawTextureEx(pause_screen, (Vector2){(float)screenWidth/2 - 760, (float)screenHeight/2 - 640}, 0, 6, WHITE);

            if ((frameCounter/20)%2) {
                DrawText("paused", camera.cam.target.x - 170, camera.cam.target.y, 50, BLACK);
            }
        } else {
            DrawTexturePro(player.texture, player.source, player.dest, (Vector2){0, 0}, 0, WHITE);
        }
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



// void checkCollisionForMouseClick() {
//
// }



int main(void) {
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messages
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "GameJam");
    // SetExitKey(KEY_NULL);

    // TODO: Load resources / Initialize variables at this point

    // Render texture to draw, enables screen scaling
    // NOTE: If screen is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    title_screen = LoadTexture("./resources/assets/title_and_pause/play_menu.png");
    pause_screen = LoadTexture("./resources/assets/title_and_pause/pause_menu.png");

    InitGame();


    //--------------------------------------------------------------------------------------
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);     // Set our game frames-per-second
                          //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) {    // Detect window close button
        UpdateDrawFrame();
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

