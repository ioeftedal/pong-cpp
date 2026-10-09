#include <iostream>
#include <string>
#include "../logger.h"
#include "raylib.h"
#include "udpserver.hpp"

enum GameState { Running, Lobby, Winner };

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

int main(void) {
  GameState gamestate = GameState::Lobby;

  const int windowWidth = 800;
  const int windowHeight = 600;

  constexpr int margin = 40;
  constexpr int paddleWidth = 20;
  constexpr int paddleHeight = 50;
  constexpr int ballSize = 10;

  int scoreP1 = 0;
  int scoreP2 = 0;

  const int paddleTop = (windowHeight - paddleHeight) / 2;

  Player player1 = {windowWidth - margin - paddleWidth, paddleTop, paddleWidth,
                    paddleHeight};

  Player player2 = {margin, paddleTop, paddleWidth, paddleHeight};

  Ball ball{windowWidth / 2, windowHeight / 2, 1, 1, ballSize, ballSize};

  Logger logger("logfile.txt", INFO);  // Create logger instance
  logger.log(INFO, "Program started.");
  logger.log(DEBUG, "Window Height: " + std::to_string(windowWidth));
  logger.log(DEBUG, "Window Height: " + std::to_string(windowHeight));

  UDPServer<> udpServer;

  udpServer.onRawMessageReceived = [&](const char* message, int length,
                                       std::string ipv4, uint16_t port) {
    std::cout << ipv4 << ":" << port << " => " << message << "(" << length
              << ")" << std::endl;

    // Echo to client:
    udpServer.SendTo(message, length, ipv4, port);
  };

  // Bind the server to a port.
  udpServer.Bind(8888, [](int errorCode, std::string errorMessage) {
    // BINDING FAILED:
    std::cout << errorCode << " : " << errorMessage << std::endl;
  });

  logger.log(INFO, "Program after UDP.");

  // TODO: There has to be 2 clients connected in order to start game
  int connected = 2;
  if (connected == 2) {
    gamestate = GameState::Running;
  }

  logger.log(INFO, "Program before getchar.");
  // You should do an input loop, so the program won't terminate immediately
  char input = getchar();
  logger.log(INFO, "Program after getchar.");

  while (gamestate == GameState::Running) {
    logger.log(INFO, "While loop entered");

    input = getchar();

    if (scoreP1 == 10) {
      gamestate = GameState::Winner;
      // TODO: This should be read off from upd client
      if (IsKeyPressed(KEY_ENTER)) {
        scoreP1 = 0;
        scoreP2 = 0;
        ball.xPos = GetScreenWidth() / 2;
        gamestate = GameState::Running;
      };
      continue;
    };
    if (scoreP2 == 10) {
      gamestate = GameState::Winner;
      // TODO: This should be read off from upd client
      if (IsKeyPressed(KEY_ENTER)) {
        scoreP1 = 0;
        scoreP2 = 0;
        ball.xPos = GetScreenWidth() / 2;
        gamestate = GameState::Running;
      };
      continue;
    };

    if (input == 'i') {
      player1.yPos -= player1.speed;
    };
    if (input == 'k') {
      player1.yPos += player1.speed;
    };
    if (input == 'e') {
      player2.yPos -= player1.speed;
    };
    if (input == 'd') {
      player2.yPos += player1.speed;
    };

    logger.log(INFO, std::to_string(player1.yPos));
    logger.log(INFO, std::to_string(input));
    std::cout << std::to_string(input);

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
  };

  udpServer.Close();
  logger.log(INFO, "Program exited.");

  return 0;
}
