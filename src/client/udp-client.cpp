#include <termios.h>
#include <iostream>
#include <string>
#include "udpsocket.hpp"

int main() {
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
    std::cout << ipv4 << ":" << port << " => " << message << "(" << length
              << ")" << std::endl;
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

  // input loop, so the program won't terminate immediately
  char input = getchar();
  while (input != '0') {
    udpSocket.Send(&input, 1);
    input = getchar();
  }

  // restore terminal settings
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  // Close the socket.
  udpSocket.Close();

  return 0;
}
