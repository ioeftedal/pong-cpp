#include <iostream>
#include "../logger.h"
#include "raylib.h"

#include <termios.h>
#include <iostream>
#include <string>
#include "udpsocket.hpp"

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

  // Our constants:
  constexpr const char* IP = "localhost";
  constexpr uint16_t PORT = 8888;

  // Initialize socket.
  UDPSocket<100> udpSocket(
      true);  // "true" to use Connection on UDP. Default is "false".
  udpSocket.Connect(IP, PORT);

  // Send String:
  udpSocket.Send("Connection started");
  //udpSocket.SendTo("ABCDEFGH", IP, PORT); // If you want to connectless

  udpSocket.onRawMessageReceived = [&](const char* message, int length,
                                       std::string ipv4, uint16_t port) {
    logger.log(INFO, (ipv4) + ":" + std::to_string(port) + " => " +
                         std::string(message, length) + "(" +
                         std::to_string(length) + ")");
  };

  // TODO: There has to be 2 clients connected in order to start game
  int connected = 2;
  if (connected == 2) {
    gamestate = GameState::Running;
  }

  // use termios to send bytes immediately
  // (not wait for enter)
  struct termios oldt, newt;
  // save current terminal setting
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  // disable canonical mode
  newt.c_lflag &= ~(ICANON | ECHO);
  // apply new setting
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  char input = getchar();

  while (gamestate == GameState::Running) {
    udpSocket.Send(&input, 1);
    input = getchar();

    if (scoreP1 == 10) {
      // gamestate = GameState::Winner;
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
      // gamestate = GameState::Winner;
      // TODO: This should be read off from upd client
      if (IsKeyPressed(KEY_ENTER)) {
        scoreP1 = 0;
        scoreP2 = 0;
        ball.xPos = GetScreenWidth() / 2;
        gamestate = GameState::Running;
      };
      continue;
    };

    // TODO: This should be read off from upd client
    // if (IsKeyDown(KEY_UP)) {
    //   player1.yPos -= player1.speed;
    // };
    // if (IsKeyDown(KEY_DOWN)) {
    //   player1.yPos += player1.speed;
    // };
    // if (IsKeyDown(KEY_W)) {
    //   player2.yPos -= player1.speed;
    // };
    // if (IsKeyDown(KEY_S)) {
    //   player2.yPos += player1.speed;
    // };

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

  // restore terminal settings
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  // Close the socket.
  udpSocket.Close();

  logger.log(INFO, "Program exited.");

  return 0;
}
