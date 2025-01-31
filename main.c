#include <math.h>
#include <stdio.h>
#include "raylib.h"
#include <stdlib.h>
#include <time.h>

typedef enum GameScreen { MENU, GAME, WIN, GAME_OVER } GameScreen;

#define screenWidth 800
#define screenHeight 480
#define blocksPerRow 10
#define blockRowCount 5
#define blockWidth (screenWidth / blocksPerRow - 20)
#define blockHeight 20
#define ballRadius 10

typedef struct {
    Rectangle rect;
    bool visible;
    int health;
} Block;

typedef struct {
    Vector2 position;
    float speedX, speedY;
    bool active;
} Ball;

typedef struct {
    Vector2 position;
    bool active;
} PowerUp;

typedef struct {
    Rectangle paddle;
    int lives;
    Ball mainBall;
    Block blocks[blockRowCount][blocksPerRow];
    Block flashingBlock;
    bool flashingBlockVisible;
    PowerUp powerUp;
    float resetTimer;
    bool isResetting;
    float ballStartDelay;
    bool ballDelayed;
} GameState;

GameState gameState;
GameScreen currentScreen = MENU;

void InitGameState() {
    gameState.lives = 1;
    gameState.paddle = (Rectangle){ screenWidth / 2 - 50, screenHeight - 50, 100, 20 };

    gameState.mainBall.position = (Vector2){ gameState.paddle.x + gameState.paddle.width / 2, gameState.paddle.y - ballRadius * 2 };
    gameState.mainBall.speedX = 0;
    gameState.mainBall.speedY = 0;
    gameState.mainBall.active = false;

    gameState.ballStartDelay = 1.0f;
    gameState.ballDelayed = true;

    gameState.resetTimer = 0;
    gameState.isResetting = false;

    srand(time(NULL));

    // Initialize all blocks
    for (int i = 0; i < blockRowCount; i++) {
        for (int j = 0; j < blocksPerRow; j++) {
            gameState.blocks[i][j].rect.x = j * (blockWidth + 20) + 10;
            gameState.blocks[i][j].rect.y = i * (blockHeight + 10) + 40;
            gameState.blocks[i][j].rect.width = blockWidth;
            gameState.blocks[i][j].rect.height = blockHeight;
            gameState.blocks[i][j].visible = true;
            gameState.blocks[i][j].health = 1;
        }
    }

    for (int k = 0; k < 5; k++) {
        int i = rand() % blockRowCount;
        int j = rand() % blocksPerRow;
        if ((i != blockRowCount / 2) || (j != blocksPerRow / 2)) { // Ensure it doesn't replace the flashing block
            gameState.blocks[i][j].health = (rand() % 2) + 2;  // Randomly assign 2 or 3 health
        }
    }

    gameState.flashingBlock.rect.x = (blocksPerRow / 2) * (blockWidth + 20) + 10;
    gameState.flashingBlock.rect.y = 40;
    gameState.flashingBlock.rect.width = blockWidth;
    gameState.flashingBlock.rect.height = blockHeight;
    gameState.flashingBlock.visible = true;
    gameState.flashingBlock.health = 1;

    gameState.powerUp.position = (Vector2){0, 0};
    gameState.powerUp.active = false;
}


void ResetBall() {
    gameState.mainBall.position = (Vector2){ gameState.paddle.x + gameState.paddle.width / 2, gameState.paddle.y - ballRadius * 2 };
    gameState.mainBall.speedX = 0;
    gameState.mainBall.speedY = 0;
    gameState.isResetting = false;
    //wait
    gameState.ballStartDelay = 1.0f;
    gameState.ballDelayed = true;
}



bool AreAllBlocksDestroyed() {
    for (int i = 0; i < blockRowCount; i++) {
        for (int j = 0; j < blocksPerRow; j++) {
            if (gameState.blocks[i][j].visible) {
                return false;
            }
        }
    }
    return true;
}

void UpdateGame(float deltaTime) {
    if (gameState.isResetting) {
        gameState.resetTimer -= deltaTime;
        if (gameState.resetTimer <= 0) {
            ResetBall();
        }
    }

    if (gameState.ballDelayed) {
        gameState.ballStartDelay -= deltaTime;
        if (gameState.ballStartDelay <= 0) {
            gameState.mainBall.speedX = (rand() % 201) - 100;
            gameState.mainBall.speedY = -500;
            gameState.mainBall.active = true;
            gameState.ballDelayed = false;
        }
    }

    if (IsKeyDown(KEY_LEFT)) gameState.paddle.x -= 400 * deltaTime;
    if (IsKeyDown(KEY_RIGHT)) gameState.paddle.x += 400 * deltaTime;
    if (gameState.paddle.x < 0) gameState.paddle.x = 0;
    if (gameState.paddle.x + gameState.paddle.width > screenWidth) gameState.paddle.x = screenWidth - gameState.paddle.width;

    if (!gameState.isResetting && !gameState.ballDelayed) {
        gameState.mainBall.position.x += gameState.mainBall.speedX * deltaTime;
        gameState.mainBall.position.y += gameState.mainBall.speedY * deltaTime;

        if (gameState.mainBall.position.x < ballRadius || gameState.mainBall.position.x > screenWidth - ballRadius) {
            gameState.mainBall.speedX *= -1;
        }

        if (gameState.mainBall.position.y < ballRadius) {
            gameState.mainBall.speedY *= -1;
        }

        if (gameState.mainBall.position.y >= screenHeight - 30 && !gameState.isResetting) {
            gameState.lives--;
            if (gameState.lives > 0) {
                ResetBall();
            } else {
                currentScreen = GAME_OVER;
            }
        }

        if (CheckCollisionCircleRec(gameState.mainBall.position, ballRadius, gameState.paddle)) {
            gameState.mainBall.speedY = -fabsf(gameState.mainBall.speedY);
            gameState.mainBall.speedX = (gameState.mainBall.position.x - (gameState.paddle.x + gameState.paddle.width / 2)) / (gameState.paddle.width / 2) * 250;
        }

        for (int i = 0; i < blockRowCount; i++) {
            for (int j = 0; j < blocksPerRow; j++) {
                if (gameState.blocks[i][j].visible && CheckCollisionCircleRec(gameState.mainBall.position, ballRadius, gameState.blocks[i][j].rect)) {
                    gameState.blocks[i][j].health--;
                    if (gameState.blocks[i][j].health <= 0) {
                        gameState.blocks[i][j].visible = false;
                    }
                    gameState.mainBall.speedY *= -1;
                }
            }
        }

        if (gameState.flashingBlock.visible && CheckCollisionCircleRec(gameState.mainBall.position, ballRadius, gameState.flashingBlock.rect)) {
            gameState.flashingBlock.visible = false;
            gameState.powerUp.position = (Vector2){ gameState.flashingBlock.rect.x + blockWidth / 2, gameState.flashingBlock.rect.y };
            gameState.powerUp.active = true;
            gameState.mainBall.speedY *= -1;
        }

        if (gameState.powerUp.active) {
            gameState.powerUp.position.y += 200 * deltaTime;
            if (gameState.powerUp.position.y >= screenHeight) {
                gameState.powerUp.active = false;
            }
        }

        // PowerCollect
        if (gameState.powerUp.active && CheckCollisionCircleRec(gameState.powerUp.position, ballRadius, gameState.paddle)) {
            gameState.lives++;
            gameState.powerUp.active = false;
        }

        if (AreAllBlocksDestroyed()) {
            currentScreen = WIN;
        }
    }
}

void DrawGame() {
    BeginDrawing();
    ClearBackground(BLACK);

    DrawRectangleRec(gameState.paddle, WHITE);
    DrawCircleV(gameState.mainBall.position, ballRadius, WHITE);

    for (int i = 0; i < blockRowCount; i++) {
        for (int j = 0; j < blocksPerRow; j++) {
            if (gameState.blocks[i][j].visible) {
                Color blockColor = (gameState.blocks[i][j].health == 1) ? YELLOW : (gameState.blocks[i][j].health == 2) ? RED : BLUE;
                DrawRectangleRec(gameState.blocks[i][j].rect, blockColor);
            }
        }
    }

    if (gameState.flashingBlock.visible) {
        Color flashingColor = (fmod(GetTime() * 10, 2) < 1) ? GREEN : RED;
        DrawRectangleRec(gameState.flashingBlock.rect, flashingColor);
    }

    if (gameState.powerUp.active) {
        DrawRectangle(gameState.powerUp.position.x - 5, gameState.powerUp.position.y - 5, 10, 10, GREEN);
    }

    DrawText(TextFormat("Lives: %d", gameState.lives), 10, screenHeight - 25, 20, WHITE);
    EndDrawing();
}

int main(void) {
    InitWindow(screenWidth, screenHeight, "Block Kuzushi wannabe type game");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        switch (currentScreen) {
            case MENU:
                BeginDrawing();
                ClearBackground(BLACK);
                DrawText("Press ENTER to Start", screenWidth / 2 - MeasureText("Press ENTER to Start", 20) / 2, screenHeight / 2 - 10, 20, WHITE);
                DrawText("Press ESC to Exit", screenWidth / 2 - MeasureText("Press ESC to Exit", 20) / 2, screenHeight / 2 + 20, 20, WHITE);
                EndDrawing();

                if (IsKeyPressed(KEY_ENTER)) {
                    InitGameState();
                    currentScreen = GAME;
                }
                break;

            case GAME:
                UpdateGame(deltaTime);
                DrawGame();
                break;

            case WIN:
                BeginDrawing();
                ClearBackground(BLACK);
                DrawText("You Win! Press R to Restart or ESC to Exit", screenWidth / 2 - MeasureText("You Win! Press R to Restart or ESC to Exit", 20) / 2, screenHeight / 2, 20, GREEN);
                EndDrawing();

                if (IsKeyPressed(KEY_R)) {
                    InitGameState();
                    currentScreen = GAME;
                } else if (IsKeyPressed(KEY_ESCAPE)) {
                    CloseWindow();
                    return 0;
                }
                break;

            case GAME_OVER:
                BeginDrawing();
                ClearBackground(BLACK);
                DrawText("Game Over! Press R to Restart", screenWidth / 2 - MeasureText("Game Over! Press R to Restart", 20) / 2, screenHeight / 2, 20, RED);
                EndDrawing();

                if (IsKeyPressed(KEY_R)) {
                    InitGameState();
                    currentScreen = GAME;
                }
                break;
        }
    }

    CloseWindow();
    return 0;
}
