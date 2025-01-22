#include <stdio.h>
#include "raylib.h"
#include <math.h>

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Block Kuzushi wannabe type game");

    Rectangle paddle = { screenWidth/2 - 50, screenHeight - 30, 100, 20 };
    Vector2 ball = { paddle.x + paddle.width / 2, paddle.y - 10 };
    float ballSpeedX = 0.0f, ballSpeedY = -300.0f;
    const int ballRadius = 10;

    const int blocksPerRow = 10, blockRowCount = 5;
    const int blockWidth = screenWidth / blocksPerRow - 20;
    const int blockHeight = 20;
    Rectangle blocks[blockRowCount][blocksPerRow];
    bool visible[blockRowCount][blocksPerRow];

    for (int i = 0; i < blockRowCount; i++) {
        for (int j = 0; j < blocksPerRow; j++) {
            blocks[i][j].x = j * (blockWidth + 20) + 10;
            blocks[i][j].y = i * (blockHeight + 10) + 40;
            blocks[i][j].width = blockWidth;
            blocks[i][j].height = blockHeight;
            visible[i][j] = true;
        }
    }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update
        float deltaTime = GetFrameTime();
        if (IsKeyDown(KEY_LEFT)) paddle.x -= 400 * deltaTime;
        if (IsKeyDown(KEY_RIGHT)) paddle.x += 400 * deltaTime;
        if (paddle.x < 0) paddle.x = 0;
        if (paddle.x + paddle.width > screenWidth) paddle.x = screenWidth - paddle.width;

        ball.x += ballSpeedX * deltaTime;
        ball.y += ballSpeedY * deltaTime;
        if (ball.x >= screenWidth - ballRadius || ball.x <= ballRadius) ballSpeedX *= -1;
        if (ball.y <= ballRadius) ballSpeedY *= -1;
        if (ball.y >= screenHeight - ballRadius) ballSpeedY = -fabs(ballSpeedY);

        if (CheckCollisionCircleRec(ball, ballRadius, paddle)) {
            ballSpeedY = -fabs(ballSpeedY);
            ballSpeedX = (ball.x - (paddle.x + paddle.width / 2)) / (paddle.width / 2) * 150;
        }

        for (int i = 0; i < blockRowCount; i++) {
            for (int j = 0; j < blocksPerRow; j++) {
                if (visible[i][j] && CheckCollisionCircleRec(ball, ballRadius, blocks[i][j])) {
                    ballSpeedY = fabs(ballSpeedY);
                    visible[i][j] = false;
                    break;
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);
        DrawRectangleRec(paddle, WHITE);
        DrawCircleV(ball, ballRadius, WHITE);

        for (int i = 0; i < blockRowCount; i++) {
            for (int j = 0; j < blocksPerRow; j++) {
                if (visible[i][j]) DrawRectangleRec(blocks[i][j], YELLOW);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
