#include "logger.h"
#include "raylib.h"

#include <termios.h>
#include <string>
#include "udpsocket.hpp"

Logger logger("logfile.txt", INFO);

const int windowWidth = 800;
const int windowHeight = 600;

int scoreP1 = 0;
int scoreP2 = 0;

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
    logger.log(INFO, (ipv4) + ":" + std::to_string(port) + " => " +
                         std::string(message, length) + "(" +
                         std::to_string(length) + ")");
  };

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
  };

  // restore terminal settings
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  // Close the socket.
  udpSocket.Close();

  logger.log(INFO, "Program exited.");

  return 0;
}
