#include <iostream>
#include "udpserver.hpp"

int main() {
  // Initialize server socket..
  UDPServer<> udpServer;

  // onMessageReceived will run when a message received with information of ip & port of sender:
  /*udpServer.onMessageReceived = [&](string message, string ipv4, uint16_t port) {
        //cout << ipv4 << ":" << port << " => " << message << endl;

        // Echo to client:
        udpServer.SendTo("A!", ipv4, port);
    };*/

  // If you want to use raw byte arrays:
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

  // You should do an input loop, so the program won't terminate immediately
  std::string input;
  getline(std::cin, input);
  while (input != "exit") {
    getline(std::cin, input);
  }

  udpServer.Close();

  return 0;
}
