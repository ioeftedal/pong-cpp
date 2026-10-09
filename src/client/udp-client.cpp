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

  // You should do an input loop, so the program won't terminate immediately
  std::string input;
  getline(std::cin, input);
  while (input != "exit") {
    udpSocket.Send(input);
    getline(std::cin, input);
  }

  // Close the socket.
  udpSocket.Close();

  return 0;
}
