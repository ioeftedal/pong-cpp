#include "common.h"
#include "logger.h"
#include "raylib.h"

#include <cstring>
#include <mutex>
#include <string>
#include "udpsocket.hpp"

static void drawPlayer(const Player& p) {
  DrawRectangle(p.xPos, p.yPos, p.width, p.height, WHITE);
}

static void drawBall(const Ball& b) {
  DrawRectangle(b.xPos, b.yPos, b.width, b.height, WHITE);
}

Logger logger("logfile.txt", DEBUG);

Information latestState{paddleTop,       paddleTop,        0,    0,
                        windowWidth / 2, windowHeight / 2, Lobby};
std::mutex stateMutex;

static void drawCenteredText(const char* text, int y, int fontSize,
                             Color color) {
  int x = (GetScreenWidth() - MeasureText(text, fontSize)) / 2;
  DrawText(text, x, y, fontSize, color);
}

int main(void) {
  logger.log(INFO, "Program started.");
  logger.log(DEBUG, "Window Height: " + std::to_string(windowWidth));
  logger.log(DEBUG, "Window Height: " + std::to_string(windowHeight));

  // Our constants:
  constexpr const char* IP = "localhost";
  constexpr uint16_t PORT = 8888;

  // Initialize socket.
  UDPSocket<100> udpSocket(true);
  udpSocket.Connect(IP, PORT);

  udpSocket.Send("Connection started");

  udpSocket.onRawMessageReceived = [&](const char* message, int length,
                                       std::string ipv4, uint16_t port) {
    if (static_cast<unsigned long>(length) < sizeof(Information))
      return;

    std::lock_guard<std::mutex> lock(stateMutex);
    memcpy(&latestState, message, sizeof(Information));
    logger.log(INFO, (ipv4) + ":" + std::to_string(port));
  };

  SetConfigFlags(FLAG_WINDOW_HIGHDPI);
  InitWindow(windowWidth, windowHeight, "PONG");
  SetTargetFPS(60);

  Player player1 = {windowWidth - margin - paddleWidth, paddleTop, paddleWidth,
                    paddleHeight};

  Player player2 = {margin, paddleTop, paddleWidth, paddleHeight};

  Ball ball{windowWidth / 2, windowHeight / 2, 1, 1, ballSize, ballSize};

  while (!WindowShouldClose()) {
    Information info;
    {
      std::lock_guard<std::mutex> lock(stateMutex);
      info = latestState;
    }

    // input: one char per held key, every frame (server consumes input each
    // tick)
    if (IsKeyDown(KEY_UP)) {
      udpSocket.Send("i", 1);
    };
    if (IsKeyDown(KEY_DOWN)) {
      udpSocket.Send("k", 1);
    };
    if (IsKeyDown(KEY_W)) {
      udpSocket.Send("e", 1);
    };
    if (IsKeyDown(KEY_S)) {
      udpSocket.Send("d", 1);
    };
    if (info.gamestate == Winner && IsKeyPressed(KEY_P)) {
      udpSocket.Send("p", 1);
    };
    // heartbeat: keep ticking so the server keeps replying with state
    udpSocket.Send("?", 1);

    BeginDrawing();
    ClearBackground(BLACK);

    if (info.gamestate == Lobby) {
      drawCenteredText("WAITING FOR PLAYERS...", 250, 40, GOLD);
      drawCenteredText("ESC to quit", 360, 20, GREEN);
      EndDrawing();
      continue;
    };
    if (info.gamestate == Winner) {
      if (info.scoreP1 == 10) {
        drawCenteredText("PLAYER 1 WINS!", 250, 40, GOLD);
      } else {
        drawCenteredText("PLAYER 2 WINS!", 250, 40, GOLD);
      };
      drawCenteredText("Press P to play again or ESC to quit", 360, 20, GREEN);
      EndDrawing();
      continue;
    };

    // positions come straight from the server packet
    player1.yPos = info.player1YPos;
    player2.yPos = info.player2YPos;
    ball.xPos = info.ballXPos;
    ball.yPos = info.ballYPos;

    drawPlayer(player1);
    drawPlayer(player2);
    drawBall(ball);

    const char* scoreTextP1 = TextFormat("Player 1: %d", info.scoreP1);
    const char* scoreTextP2 = TextFormat("Player 2: %d", info.scoreP2);

    drawCenteredText("PONG", 20, 40, WHITE);
    DrawText(scoreTextP2, margin, 20, 20, WHITE);
    DrawText(scoreTextP1,
             GetScreenWidth() - margin - MeasureText(scoreTextP1, 20), 20, 20,
             WHITE);
    EndDrawing();
  };

  CloseWindow();

  // Close the socket.
  udpSocket.Close();

  logger.log(INFO, "Program exited.");

  return 0;
}
