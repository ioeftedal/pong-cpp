#include <chrono>
#include <thread>
#include <vector>
#include "logger.h"
#include "udpserver.hpp"

Logger logger("logfile.txt", DEBUG);

const int windowWidth = 800;
const int windowHeight = 600;

constexpr int margin = 40;
constexpr int paddleWidth = 20;
constexpr int paddleHeight = 50;
constexpr int ballSize = 10;

int scoreP1 = 0;
int scoreP2 = 0;

char player1Input = '\0';
char player2Input = '\0';

enum GameState { Running, Lobby, Winner };

struct Client {
  std::string ipv4;
  int port;
};

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

  void update() {
    if (yPos < 0) {
      yDir *= -1;
    };
    if (yPos + height > windowHeight) {
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
  logger.log(INFO, "Program started.");

  // Initial gamestate
  GameState gamestate = GameState::Lobby;

  // Collection of clients that has connected to the server
  std::vector<Client> udp_clients;

  const int paddleTop = (windowHeight - paddleHeight) / 2;

  Player player1 = {windowWidth - margin - paddleWidth, paddleTop, paddleWidth,
                    paddleHeight};

  Player player2 = {margin, paddleTop, paddleWidth, paddleHeight};

  Ball ball{windowWidth / 2, windowHeight / 2, 1, 1, ballSize, ballSize};

  // Initialise udp server
  UDPServer<> udpServer;

  // Whenever a message is recieved
  udpServer.onRawMessageReceived = [&](const char* message, int length,
                                       std::string ipv4, uint16_t port) {
    // add a client to vector if client is not already in vector
    Client udp_client = {ipv4, port};
    bool unique_client = true;
    for (Client client : udp_clients) {
      if (udp_client.port == client.port && udp_client.ipv4 == client.ipv4) {
        unique_client = false;
      };
    };
    if (unique_client) {
      udp_clients.push_back(udp_client);
    }

    // if message from client not empty, set it as player input
    if (length <= 0) {
      player1Input = '\0';
      player2Input = '\0';
      return;
    }
    char input = message[0];

    // TODO: Identify the player by their registered client endpoint.
    if (input == 'i' || input == 'k') {
      player1Input = input;
    } else if (input == 'e' || input == 'd') {
      player2Input = input;
    }

    // Log message and info
    logger.log(INFO, (ipv4) + ":" + std::to_string(port) + " => " +
                         std::string(message, length) + "(" +
                         std::to_string(length) + ")");

    // TODO: send back more information.
    // A struct?
    // need to send player positions, scores, ball position and gamestate
    udpServer.SendTo(message, length, ipv4, port);
  };

  // Bind the server to a port.
  udpServer.Bind(8888, [](int errorCode, std::string errorMessage) {
    logger.log(ERROR, std::to_string(errorCode) + " : " + errorMessage + "\n");
  });

  // cap at 60 fps
  using Clock = std::chrono::steady_clock;
  constexpr auto frameDuration = std::chrono::nanoseconds(1'000'000'000 / 60);

  // main game loop
  while (true) {
    switch (gamestate) {
      case Running: {
        const auto frameStart = Clock::now();

        std::string udp_clients_info = "UDP Clients: ";
        for (const auto& client : udp_clients) {
          udp_clients_info +=
              client.ipv4 + ":" + std::to_string(client.port) + "   ";
        }
        logger.log(INFO, udp_clients_info);

        // Winner logic
        if (scoreP1 == 10) {
          gamestate = GameState::Winner;
          logger.log(DEBUG, "Player 1 is the winner.");
        };
        if (scoreP2 == 10) {
          gamestate = GameState::Winner;
          logger.log(DEBUG, "Player 2 is the winner.");
        };

        // Movement of player logic
        if (player1Input == 'i') {
          player1.yPos -= player1.speed;
        };
        if (player1Input == 'k') {
          player1.yPos += player1.speed;
        };
        if (player2Input == 'e') {
          player2.yPos -= player1.speed;
        };
        if (player2Input == 'd') {
          player2.yPos += player1.speed;
        };
        player1Input = '\0';
        player2Input = '\0';
        // lock player at top and bottom of screen
        if (player1.yPos < 0) {
          player1.yPos = 0;
        };
        if (player1.yPos + player1.height > windowHeight) {
          player1.yPos = windowHeight - player1.height;
        };
        if (player2.yPos < 0) {
          player2.yPos = 0;
        };
        if (player2.yPos + player2.height > windowHeight) {
          player2.yPos = windowHeight - player2.height;
        };
        logger.log(DEBUG, "Player 1 position: " + std::to_string(player1.yPos));
        logger.log(DEBUG, "Player 2 position: " + std::to_string(player2.yPos));

        // Ball movement logic
        if (ball.getRect().intersects(player1.getRect()) ||
            ball.getRect().intersects(player2.getRect())) {
          ball.xDir *= -1;
        };
        if (ball.xPos <= 0) {
          scoreP1 += 1;
          ball.xPos = windowWidth / 2;
        };
        if (ball.xPos + ball.width >= windowWidth) {
          scoreP2 += 1;
          ball.xPos = windowWidth / 2;
        };
        ball.update();
        logger.log(DEBUG, "Ball X Position: " + std::to_string(ball.xPos));
        logger.log(DEBUG, "Ball Y Position: " + std::to_string(ball.yPos));

        std::this_thread::sleep_until(frameStart + frameDuration);
        break;
      }
      case Lobby:
        // TODO: make this async
        logger.log(DEBUG, "In Lobby");
        if (udp_clients.size() >= 2) {
          gamestate = Running;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        break;
      case Winner:
        if (player1Input == 'p' || player2Input == 'p') {
          scoreP1 = 0;
          scoreP2 = 0;
          ball.xPos = windowWidth / 2;
          gamestate = GameState::Running;
        };
        break;
    };
  }
  udpServer.Close();
  logger.log(INFO, "Program exited.");

  return 0;
}
