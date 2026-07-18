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
#include <stddef.h>

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

#define MAX_BEE 16
#define MAX_FAKE_BEE 16

#define FIRST_WAVE_BEE 4
#define FIRST_WAVE_FAKE_BEE 4

#define SECOND_WAVE_BEE 8
#define SECOND_WAVE_FAKE_BEE 8

#define THIRD_WAVE_BEE 16
#define THIRD_WAVE_FAKE_BEE 10

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

    // player body
    Rectangle hitbox;
    Vector2 old_pos;
} Player;

typedef struct {
    Rectangle rect;
    Vector2 speed;
    bool active;
    Color color;
} Bee;

typedef struct {
    Rectangle rect;
    Vector2 speed;
    bool active;
    Color color;
} FakeBee;

typedef enum {
    First = 0,
    Second,
    Third,
} BeeWave;

// TODO: Define your custom data types here
//global state for pausing the game
static const int screenWidth = 720;
static const int screenHeight = 720;

static Vector2 mousePos = {0, 0};

static Player player = { 0 };
static int score = 0;
static bool gameover = false;
static bool first_time = true;
static bool paused = false;
static bool victory = false;

static Bee bee[MAX_BEE] = {0};
static FakeBee fake_bee[MAX_FAKE_BEE] = {0};
static BeeWave wave = {0};

static float alpha = 0;
static int active_bees = 0;
static int active_fake_bees = 0;

static bool good_bees_caught = false;
static int fake_bees_caught = 0;
static bool smooth = false;

Texture2D title_screen;
Texture2D pause_screen;

static RenderTexture2D target = {0};  // Render texture to render our game
static int frameCounter = 0;

static Rectangle hard_rects[4] = {0};
static size_t size_of_rects = 0;

static float circle_radius = 20;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame
static void InitGame();
static void drawTiles();
static void UpdateGame();
static void UpdatePlayer();
static Vector2 Normalize(Vector2 dir);
static void collider_for_x(Player *player, Rectangle hard_rects[], size_t size_of_rects);
static void collider_for_y(Player *player, Rectangle hard_rects[], size_t size_of_rects);

//--------------------------------------------------------------------------------------------
// Module Functions Definition
//--------------------------------------------------------------------------------------------

void InitGame() {
    paused = false;
    gameover = false;
    victory = false;
    smooth = false;
    wave = First;

    active_bees = FIRST_WAVE_BEE;
    active_fake_bees = FIRST_WAVE_FAKE_BEE;

    good_bees_caught = false;
    fake_bees_caught = 0;

    score = 0;
    alpha = 0;

    // initialize bees and fake_bees
    for (int i = 0; i < MAX_BEE; i++) {
        bee[i].rect.width = 32.0 * 1.5;
        bee[i].rect.height = 32.0 * 1.5;

        bee[i].rect.x = GetRandomValue(screenWidth, screenWidth + 1000);
        bee[i].rect.y = GetRandomValue(0, screenHeight - fake_bee[i].rect.height);

        bee[i].speed.x = 5;
        bee[i].speed.y = 5;
        bee[i].active = true;
        bee[i].color = YELLOW;
    }

    for (int i = 0; i < MAX_FAKE_BEE; i++) {
        fake_bee[i].rect.width = 32.0 * 1.5;
        fake_bee[i].rect.height = 32.0 * 1.5;

        fake_bee[i].rect.x = GetRandomValue(screenWidth, screenWidth + 1000);
        fake_bee[i].rect.y = GetRandomValue(0, screenHeight - fake_bee[i].rect.height);

        fake_bee[i].speed.x = 5;
        fake_bee[i].speed.y = 5;
        fake_bee[i].active = true;
        fake_bee[i].color = RED;
    }

    Rectangle boundary1;
        boundary1.x = 0;
        boundary1.y = 0;
        boundary1.width = 5;
        boundary1.height = 720;

    Rectangle boundary2;
        boundary2.x = 0;
        boundary2.y = 0;
        boundary2.width = 720;
        boundary2.height = 5;

     Rectangle boundary3;
        boundary3.x = 715;
        boundary3.y = 0;
        boundary3.width = 5;
        boundary3.height = 720;

     Rectangle boundary4;
        boundary4.x = 0;
        boundary4.y = 715;
        boundary4.width = 720;
        boundary4.height = 5;

    hard_rects[0] = boundary1;
    hard_rects[1] = boundary2;
    hard_rects[2] = boundary3;
    hard_rects[3] = boundary4;

    size_of_rects = sizeof(hard_rects) / sizeof(hard_rects[0]);

    // Initialize player
    player.player_health_points = 5,
    player.speed = 300,
    player.texture = LoadTexture("./resources/assets/player/sqPlayer2.png"),
    player.player_is_hit = false,

    player.player_pos.x = (float)screenWidth / 2 - 50;
    player.player_pos.y = (float)screenHeight / 2 - 50;

    player.num_frame = 4,
    player.cur_frame = 0,

    // source refers to -> from where to start drawing
    player.source.x = 0.0;
        player.source.y = 0.0;
        player.source.width = 32.0;
        player.source.height = 32.0;

        player.dest.width = 32.0 * 2;
        player.dest.height = 32.0 * 2;

        player.hitbox.width = 32.0;
        player.hitbox.height = 54.0;
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
            player->hitbox.x = player->old_pos.x + 16;
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
            player->hitbox.y = player->old_pos.y + 10;
            // printf("\n\n                Collided\n\n");
        } else {
            // printf("\n");
        }
    }
}

void UpdateGame() {
    if (!gameover) {
        switch(wave) {
            case First:
            {
                if (!smooth)
                {
                    alpha += 0.02f;
                    if (alpha >= 1.0f) smooth = true;
                }

                if (smooth) alpha -= 0.02f;

                if (fake_bees_caught == active_fake_bees + 2)
                {
                    fake_bees_caught = 0;

                    for (int i = 0; i < active_fake_bees; i++)
                    {
                        if (!fake_bee[i].active) fake_bee[i].active = true;
                    }

                    active_fake_bees = SECOND_WAVE_FAKE_BEE;
                    wave = Second;
                    smooth = false;
                    alpha = 0.0f;
                }
                if (!good_bees_caught) {
                    for (int i = 0; i < active_bees; i++)
                    {
                        if (!bee[i].active) bee[i].active = true;
                    }

                    active_bees = SECOND_WAVE_BEE;
                }
            } break;

            case Second:
            {
                if (!smooth)
                {
                    alpha += 0.02f;

                    if (alpha >= 1.0f) smooth = true;
                }

                if (smooth) alpha -= 0.02f;

                if (fake_bees_caught == active_fake_bees + 4)
                {
                    fake_bees_caught = 0;

                    for (int i = 0; i < active_fake_bees; i++)
                    {
                        if (!fake_bee[i].active) fake_bee[i].active = true;
                    }

                    active_fake_bees = THIRD_WAVE_FAKE_BEE;
                    wave = Third;
                    smooth = false;
                    alpha = 0.0f;
                }
                if (!good_bees_caught) {
                    for (int i = 0; i < active_bees; i++)
                    {
                        if (!bee[i].active) bee[i].active = true;
                    }

                    active_bees = THIRD_WAVE_BEE - 8;
                }
            } break;

            case Third:
            {
                if (!smooth)
                {
                    alpha += 0.02f;

                    if (alpha >= 1.0f) smooth = true;
                }

                if (smooth) alpha -= 0.02f;

                if (fake_bees_caught == active_fake_bees + 8) victory = true;

            } break;

            default: break;
        }

        // Player collision with bee
        for (int i = 0; i < active_fake_bees; i++)
        {
            if (CheckCollisionRecs(player.hitbox, fake_bee[i].rect)) gameover = true;
            if (CheckCollisionRecs(player.hitbox, bee[i].rect)) gameover = true;
        }

        // fake_bee behaviour
        for (int i = 0; i < active_fake_bees; i++)
        {
            if (fake_bee[i].active)
            {
                fake_bee[i].rect.x -= fake_bee[i].speed.x;

                if (fake_bee[i].rect.x < 0)
                {
                    fake_bee[i].rect.x = GetRandomValue(screenWidth, screenWidth + 1000);
                    fake_bee[i].rect.y = GetRandomValue(0, screenHeight - fake_bee[i].rect.height);
                }
            }
        }

        for (int i = 0; i < active_bees; i++)
        {
            if (bee[i].active)
            {
                bee[i].rect.x -= bee[i].speed.x;

                if (bee[i].rect.x < 0)
                {
                    bee[i].rect.x = GetRandomValue(screenWidth, screenWidth + 1000);
                    bee[i].rect.y = GetRandomValue(0, screenHeight - bee[i].rect.height);
                }
            }
        }

        for (int i = 0; i < active_fake_bees; ++i) {
            if (CheckCollisionCircleRec(mousePos, circle_radius, fake_bee[i].rect)) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    fake_bee[i].rect.x = GetRandomValue(screenWidth, screenWidth + 1000);
                    fake_bee[i].rect.y = GetRandomValue(0, screenHeight - fake_bee[i].rect.height);
                    ++fake_bees_caught;
                    score += 100;
                }
            }
        }

        for (int i = 0; i < active_bees; ++i) {
            if (CheckCollisionCircleRec(mousePos, circle_radius, bee[i].rect)) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    gameover = true;
                }
            }
        }
    } else {
        if (IsKeyPressed(KEY_ENTER)) {
            InitGame();
            gameover = false;
        }
    }
}

void UpdatePlayer() {
    mousePos = GetMousePosition();
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

    float dt = GetFrameTime();

    player.old_pos = player.player_pos;
    Vector2 norm_dir;
    norm_dir = Normalize(dir);

    player.player_pos.x += norm_dir.x * player.speed * dt;
    player.hitbox.x = player.player_pos.x + 16;
    collider_for_x(&player, hard_rects, size_of_rects);

    player.player_pos.y += norm_dir.y * player.speed * dt;
    player.hitbox.y = player.player_pos.y + 10;
    collider_for_y(&player, hard_rects, size_of_rects);


    player.dest.x = player.player_pos.x;
    player.dest.y = player.player_pos.y;
    printf("\n\n%d\n\n", (int)player.player_pos.x);
    printf("\n\n%d\n\n", (int)player.player_pos.y);
}

void drawTiles() {
    float SposY = 0;

    float posX = 0;
    float posY = 0;

    float radius = 30;
    float inradius = (radius * sqrt(3)) / 2;

    int width = 32;
    int height = 32;

    for (int i = 0; i < width; ++i) {
        if (i % 2 == 0) {
            posY = SposY;
        } else {
            posY = SposY + inradius;
        }
        for (int j = 0; j < height; ++j) {
            DrawPoly((Vector2){posX, posY}, 6, radius, 0, (Color){235, 169, 55, 255});
            DrawPolyLinesEx((Vector2){posX, posY}, 6, radius, 0, 1, BLACK);
            posY += inradius * 2;
        }
        posX += radius + (int)(radius/2);
    }
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

    if (!paused && !first_time) {
        UpdatePlayer();
        UpdateGame();
    } else {
        frameCounter++;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
        drawTiles();
        if (!gameover) {
            if (first_time) {
                BeginDrawing();
                DrawTextureEx(title_screen, (Vector2){(float)screenWidth/2 - 320, (float)screenHeight/2 - 340}, 0, 5, WHITE);
                if (IsKeyDown(KEY_SPACE)) {
                    first_time = false;
                }
            } else if (!first_time) {
                if (paused) {
                    if ((frameCounter/20)%2) {
                        DrawText("paused", screenWidth/2, screenHeight/2, 50, BLACK);
                    }
                } else {
                    if (wave == First) DrawText("FIRST WAVE", screenWidth/2 - MeasureText("FIRST WAVE", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
                    else if (wave == Second) DrawText("SECOND WAVE", screenWidth/2 - MeasureText("SECOND WAVE", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
                    else if (wave == Third) DrawText("THIRD WAVE", screenWidth/2 - MeasureText("THIRD WAVE", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));

                    DrawTexturePro(player.texture, player.source, player.dest, (Vector2){0, 0}, 0, WHITE);
                    DrawCircleLinesV(mousePos, circle_radius, WHITE);
                    // DrawRectangleLinesEx(player.hitbox, 2, BLUE);
                    // DrawRectangleLinesEx(hard_rects[0], 3, RED);
                    // DrawRectangleLinesEx(hard_rects[1], 3, RED);
                    // DrawRectangleLinesEx(hard_rects[2], 3, RED);
                    // DrawRectangleLinesEx(hard_rects[3], 3, RED);

                    for (int i = 0; i < active_fake_bees; i++) {
                        if (fake_bee[i].active) DrawRectangleRec(fake_bee[i].rect, fake_bee[i].color);
                    }

                    for (int i = 0; i < active_bees; i++) {
                        if (bee[i].active) DrawRectangleRec(bee[i].rect, bee[i].color);
                    }

                    DrawText(TextFormat("%04i", score), 20, 20, 40, GRAY);

                    if (victory) DrawText("YOU WIN", screenWidth/2 - MeasureText("YOU WIN", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
                }
            }
        } else {
            DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);
        }

    EndDrawing();
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------

int main(void) {
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messages
#endif

    // Initialization
    InitWindow(screenWidth, screenHeight, "Conquer The Hive");
    // SetExitKey(KEY_NULL);

    target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    title_screen = LoadTexture("./resources/assets/menu/play_menu.png");
    pause_screen = LoadTexture("./resources/assets/menu/pause_menu.png");

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
    UnloadRenderTexture(target);
    UnloadTexture(title_screen);
    UnloadTexture(pause_screen);
    UnloadTexture(player.texture);

    CloseWindow();        // Close window and OpenGL context

    return 0;
}

