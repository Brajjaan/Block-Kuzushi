#include <math.h>
#include <stdio.h>
#include "raylib.h"
#include <stdlib.h> // Include for rand() and srand()
#include <time.h>   // Include for time()

typedef enum GameScreen { MENU, GAME, GAME_OVER } GameScreen;

#define screenWidth 800
#define screenHeight 450
#define blocksPerRow 10
#define blockRowCount 5
#define blockWidth (screenWidth / blocksPerRow - 20)
#define blockHeight 20

Rectangle blocks[blockRowCount][blocksPerRow];
int blockHealth[blockRowCount][blocksPerRow];
bool visible[blockRowCount][blocksPerRow];
Rectangle paddle;
Vector2 ball, extraBall;
float ballSpeedX, ballSpeedY, extraBallSpeedX, extraBallSpeedY;
const int ballRadius = 10;

int specialBlockRow, specialBlockCol;
bool specialBlockVisible;
bool extraBallActive;

void InitGameState() {
    paddle = (Rectangle){ screenWidth/2 - 50, screenHeight - 30, 100, 20 };
    ball = (Vector2){ paddle.x + paddle.width / 2, paddle.y - 10 };
    ballSpeedY = -500.0f;
    ballSpeedX = (rand() % 101) - 50;

    srand(time(NULL)); // Initialize random seed
    for (int i = 0; i < blockRowCount; i++) {
        for (int j = 0; j < blocksPerRow; j++) {
            blocks[i][j].x = j * (blockWidth + 20) + 10;
            blocks[i][j].y = i * (blockHeight + 10) + 40;
            blocks[i][j].width = blockWidth;
            blocks[i][j].height = blockHeight;
            visible[i][j] = true;
            blockHealth[i][j] = 1; // Default health is 1
        }
    }

    for (int k = 0; k < 5; k++) {
        int i = rand() % blockRowCount;
        int j = rand() % blocksPerRow;
        blockHealth[i][j] = (rand() % 2) + 2;
    }

    // Extraball pos
    int rowsFromBottom[] = {1, 2};
    specialBlockRow = blockRowCount - 1 - rowsFromBottom[rand() % 2];
    specialBlockCol = rand() % blocksPerRow;
    specialBlockVisible = true;

    extraBallActive = false;
}

int main(void) {
    InitWindow(screenWidth, screenHeight, "Block Kuzushi wannabe type game");
    SetTargetFPS(60);

    GameScreen currentScreen = MENU;

    while (!WindowShouldClose()) {
        switch (currentScreen) {
            case MENU:
                if (IsKeyPressed(KEY_ENTER)) {
                    InitGameState();
                    currentScreen = GAME;
                }
                if (IsKeyPressed(KEY_ESCAPE)) {
                    CloseWindow();
                }
                break;

            case GAME:
                float deltaTime = GetFrameTime();
                if (IsKeyDown(KEY_LEFT)) paddle.x -= 400 * deltaTime;
                if (IsKeyDown(KEY_RIGHT)) paddle.x += 400 * deltaTime;
                if (paddle.x < 0) paddle.x = 0;
                if (paddle.x + paddle.width > screenWidth) paddle.x = screenWidth - paddle.width;

                ball.x += ballSpeedX * deltaTime;
                ball.y += ballSpeedY * deltaTime;
                if (ball.x <= 0 + ballRadius || ball.x >= screenWidth - ballRadius) ballSpeedX *= -1;
                if (ball.y <= ballRadius) ballSpeedY *= -1;
                if (ball.y >= screenHeight) currentScreen = GAME_OVER;

                if (extraBallActive) {
                    extraBall.x += extraBallSpeedX * deltaTime;
                    extraBall.y += extraBallSpeedY * deltaTime;
                    if (extraBall.x <= 0 + ballRadius || extraBall.x >= screenWidth - ballRadius) extraBallSpeedX *= -1;
                    if (extraBall.y <= ballRadius) extraBallSpeedY *= -1;
                    if (extraBall.y >= screenHeight) extraBallActive = false;
                }

                if (CheckCollisionCircleRec(ball, ballRadius, paddle)) {
                    ballSpeedY = -fabsf(ballSpeedY);
                    ballSpeedX = (ball.x - (paddle.x + paddle.width / 2)) / (paddle.width / 2) * 250;
                }

                if (extraBallActive && CheckCollisionCircleRec(extraBall, ballRadius, paddle)) {
                    extraBallSpeedY = -fabsf(extraBallSpeedY);
                }

                // Collisions with blocks for both balls
                for (int i = 0; i < blockRowCount; i++) {
                    for (int j = 0; j < blocksPerRow; j++) {
                        if (visible[i][j]) {
                            if (CheckCollisionCircleRec(ball, ballRadius, blocks[i][j])) {
                                blockHealth[i][j]--;
                                if (blockHealth[i][j] <= 0) {
                                    visible[i][j] = false;
                                    if (i == specialBlockRow && j == specialBlockCol) {
                                        extraBallActive = true;
                                        extraBall = (Vector2){ blocks[i][j].x + blockWidth / 2, blocks[i][j].y + blockHeight };
                                        extraBallSpeedX = (rand() % 101) - 50;
                                        extraBallSpeedY = 200;
                                    }
                                }
                                ballSpeedY = -ballSpeedY;
                            }
                            if (extraBallActive && CheckCollisionCircleRec(extraBall, ballRadius, blocks[i][j])) {
                                blockHealth[i][j]--;
                                if (blockHealth[i][j] <= 0) visible[i][j] = false;
                                extraBallSpeedY = -extraBallSpeedY;
                            }
                        }
                    }
                }

                break;

            case GAME_OVER:
                if (IsKeyPressed(KEY_R)) {
                    InitGameState();
                    currentScreen = GAME;
                }
                if (IsKeyPressed(KEY_ESCAPE)) {
                    currentScreen = MENU;
                }
                break;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        switch (currentScreen) {
            case MENU:
                DrawText("Press ENTER to Start", screenWidth / 2 - MeasureText("Press ENTER to Start", 20) / 2, screenHeight / 2 - 10, 20, WHITE);
                DrawText("Press ESC to Exit", screenWidth / 2 - MeasureText("Press ESC to Exit", 20) / 2, screenHeight / 2 + 20, 20, WHITE);
                break;
            case GAME:
                DrawRectangleRec(paddle, WHITE);
                DrawCircleV(ball, ballRadius, WHITE);
                if (extraBallActive) {
                    DrawCircleV(extraBall, ballRadius, BLUE);
                }
                for (int i = 0; i < blockRowCount; i++) {
                    for (int j = 0; j < blocksPerRow; j++) {
                        if (visible[i][j]) {
                            Color blockColor = YELLOW;
                            if (blockHealth[i][j] == 2) blockColor = RED;
                            else if (blockHealth[i][j] == 3) blockColor = BLUE;
                            DrawRectangleRec(blocks[i][j], blockColor);

                            if (i == specialBlockRow && j == specialBlockCol && visible[i][j]) {
                                if (((int)(GetTime() * 10) % 2) == 0) {
                                    DrawRectangleLinesEx(blocks[i][j], 2, MAGENTA);
                                }
                            }
                        }
                    }
                }
                break;
            case GAME_OVER:
                DrawText("Game Over! Press R to Restart or ESC to Exit", screenWidth / 2 - MeasureText("Game Over! Press R to Restart or ESC to Return to Menu", 20) / 2, screenHeight / 2 - 10, 20, RED);
                break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}