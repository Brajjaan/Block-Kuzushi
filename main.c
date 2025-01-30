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
int blockHealth[blockRowCount][blocksPerRow]; // Array to track health of each block
bool visible[blockRowCount][blocksPerRow];
Rectangle paddle;
Vector2 ball;
float ballSpeedX, ballSpeedY;
const int ballRadius = 10;

void InitGameState() {
    paddle = (Rectangle){ screenWidth/2 - 50, screenHeight - 30, 100, 20 };
    ball = (Vector2){ paddle.x + paddle.width / 2, paddle.y - 10 };
    ballSpeedX = 0.0f;
    ballSpeedY = -300.0f;

    srand(time(NULL)); // Seed for random number generation
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

    // Assign higher health to 5 random blocks
    for (int k = 0; k < 5; k++) {
        int i = rand() % blockRowCount;
        int j = rand() % blocksPerRow;
        if (blockHealth[i][j] == 1) { // Ensure the block doesn't already have increased health
            blockHealth[i][j] = (k % 2) + 2; // Randomly assign health of 2 or 3
        }
    }
}

int main(void) {
    InitWindow(screenWidth, screenHeight, "Block Kuzushi wannabe type game");

    GameScreen currentScreen = MENU;
    SetTargetFPS(60);

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
                if (ball.x >= screenWidth - ballRadius || ball.x <= ballRadius) ballSpeedX *= -1;
                if (ball.y <= ballRadius) ballSpeedY *= -1;

                if (ball.y >= screenHeight) {
                    currentScreen = GAME_OVER;
                }

                if (CheckCollisionCircleRec(ball, ballRadius, paddle)) {
                    ballSpeedY = -fabsf(ballSpeedY);
                    ballSpeedX = (ball.x - (paddle.x + paddle.width / 2)) / (paddle.width / 2) * 150;
                }

                for (int i = 0; i < blockRowCount; i++) {
                    for (int j = 0; j < blocksPerRow; j++) {
                        if (visible[i][j] && CheckCollisionCircleRec(ball, ballRadius, blocks[i][j])) {
                            blockHealth[i][j]--;
                            if (blockHealth[i][j] <= 0) {
                                visible[i][j] = false;
                            }
                            ballSpeedY = fabsf(ballSpeedY);
                            break;
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
                for (int i = 0; i < blockRowCount; i++) {
                    for (int j = 0; j < blocksPerRow; j++) {
                        if (visible[i][j]) {
                            Color blockColor = YELLOW; // Default color for health 1
                            if (blockHealth[i][j] == 2) blockColor = RED; // Red for health 2
                            else if (blockHealth[i][j] == 3) blockColor = BLUE; // Blue for health 3
                            DrawRectangleRec(blocks[i][j], blockColor);
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
