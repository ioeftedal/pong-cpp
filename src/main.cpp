#include "logger.h"
#include "raylib.h"

struct Rect {
  int x;
  int y;
  int width;
  int height;

  bool intersects(const Rect& other) {
    return x < other.x + other.width && x + width > other.x &&
           y < other.y + other.height && y + height > other.y;
  };
};

struct Player {
  int xPos;
  int yPos;
  int width;
  int height;
  int speed = 10;

  void draw() const { DrawRectangle(xPos, yPos, width, height, WHITE); };

  Rect getRect() {
    return {
        xPos,
        yPos,
        width,
        height,
    };
  };
};

struct Ball {
  int xPos;
  int yPos;
  int xDir;
  int yDir;
  int width;
  int height;
  int speed = 3;

  void draw() const { DrawRectangle(xPos, yPos, width, height, WHITE); };

  void update() {
    if (yPos < 0) {
      yDir *= -1;
    };
    if (yPos + height > GetScreenHeight()) {
      yDir *= -1;
    };
    yPos += yDir * speed;
    xPos += xDir * speed;
  };

  Rect getRect() {
    return {
        xPos,
        yPos,
        width,
        height,
    };
  };
};

static void drawCenteredText(const char* text, int y, int fontSize,
                             Color color) {
  int x = (GetScreenWidth() - MeasureText(text, fontSize)) / 2;
  DrawText(text, x, y, fontSize, color);
}

int main(void) {
  Logger logger("logfile.txt");  // Create logger instance
  logger.log(INFO, "Program started.");
  const int windowWidth = 800;
  const int windowHeight = 600;

  constexpr int margin = 40;
  constexpr int paddleWidth = 20;
  constexpr int paddleHeight = 50;
  constexpr int ballSize = 10;

  SetConfigFlags(FLAG_WINDOW_HIGHDPI);
  InitWindow(windowWidth, windowHeight, "PONG");
  SetTargetFPS(60);

  int scoreP1 = 0;
  int scoreP2 = 0;

  const int paddleTop = (windowHeight - paddleHeight) / 2;

  Player player1 = {windowWidth - margin - paddleWidth, paddleTop, paddleWidth,
                    paddleHeight};

  Player player2 = {margin, paddleTop, paddleWidth, paddleHeight};

  Ball ball{windowWidth / 2, windowHeight / 2, 1, 1, ballSize, ballSize};

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    if (scoreP1 == 10) {
      drawCenteredText("PLAYER 1 WINS!", 250, 40, GOLD);
      drawCenteredText("Press ENTER to play again or ESC to quit", 360, 20,
                       GREEN);
      if (IsKeyPressed(KEY_ENTER)) {
        scoreP1 = 0;
        scoreP2 = 0;
        ball.xPos = GetScreenWidth() / 2;
      };
      EndDrawing();
      continue;
    };
    if (scoreP2 == 10) {
      drawCenteredText("PLAYER 2 WINS!", 250, 40, GOLD);
      drawCenteredText("Press ENTER to play again or ESC to quit", 360, 20,
                       GREEN);
      if (IsKeyPressed(KEY_ENTER)) {
        scoreP1 = 0;
        scoreP2 = 0;
        ball.xPos = GetScreenWidth() / 2;
      };
      EndDrawing();
      continue;
    };

    player1.draw();
    player2.draw();

    if (IsKeyDown(KEY_UP)) {
      player1.yPos -= player1.speed;
    };
    if (IsKeyDown(KEY_DOWN)) {
      player1.yPos += player1.speed;
    };
    if (IsKeyDown(KEY_W)) {
      player2.yPos -= player1.speed;
    };
    if (IsKeyDown(KEY_S)) {
      player2.yPos += player1.speed;
    };

    if (player1.yPos < 0) {
      player1.yPos = 0;
    };
    if (player1.yPos + player1.height > GetScreenHeight()) {
      player1.yPos = GetScreenHeight() - player1.height;
    };
    if (player2.yPos < 0) {
      player2.yPos = 0;
    };
    if (player2.yPos + player2.height > GetScreenHeight()) {
      player2.yPos = GetScreenHeight() - player2.height;
    };

    ball.draw();
    if (ball.getRect().intersects(player1.getRect()) ||
        ball.getRect().intersects(player2.getRect())) {
      ball.xDir *= -1;
    };
    if (ball.xPos <= 0) {
      scoreP1 += 1;
      ball.xPos = GetScreenWidth() / 2;
    };
    if (ball.xPos + ball.width >= GetScreenWidth()) {
      scoreP2 += 1;
      ball.xPos = GetScreenWidth() / 2;
    };
    ball.update();

    const char* scoreTextP1 = TextFormat("Player 1: %d", scoreP1);
    const char* scoreTextP2 = TextFormat("Player 2: %d", scoreP2);

    drawCenteredText("PONG", 20, 40, WHITE);
    DrawText(scoreTextP2, margin, 20, 20, WHITE);
    DrawText(scoreTextP1,
             GetScreenWidth() - margin - MeasureText(scoreTextP1, 20), 20, 20,
             WHITE);
    EndDrawing();
  };

  CloseWindow();

  return 0;
}
