inline const int windowWidth = 800;
inline const int windowHeight = 600;

inline constexpr int margin = 40;
inline constexpr int paddleWidth = 20;
inline constexpr int paddleHeight = 50;
inline constexpr int ballSize = 10;

inline const int paddleTop = (windowHeight - paddleHeight) / 2;

enum GameState { Running, Lobby, Winner };

struct Information {
  int player1YPos;
  int player2YPos;
  int scoreP1;
  int scoreP2;
  int ballXPos;
  int ballYPos;
  GameState gamestate;
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
