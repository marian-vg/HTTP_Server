#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <winsock2.h>
#include <ws2tcpip.h>

int main(int argc, char **argv) {

  // [1] Initialize Winsock

  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    std::cerr << "WSAStartup failed\n";
    return 1;
  }

  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  // [2] Create a socket

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    WSACleanup();
    return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors

  // [3] Reuse the address

  int reuse = 1;

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  // [4] Bind the socket to the port

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {

    // The bind function returns SOCKET_ERROR on failure, not -1 like in Linux

    std::cerr << "Failed to bind to port 4221\n";
    closesocket(server_fd);
    WSACleanup();
    return 1;
  }

  // [5] Listen for incoming connections

  int connection_backlog = 5;

  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    closesocket(server_fd);
    WSACleanup();
    return 1;
  }

  std::cout << "Server listening on port 4221...\n";

  // [6] Accept incoming connections

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  // Store the value returned by accept in a variable
  auto client = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);

  if (client == INVALID_SOCKET) {
    std::cerr << "accept failed\n";
    closesocket(server_fd);
    WSACleanup();
    return 1;
  }

  std::cout << "Client connected\n";

  // [7] Receive data from the client

  // Create a buffer to store the client's message
  char buffer[1024] = {0};

  int bytes_received = recv(client, buffer, sizeof(buffer), 0);

  if (bytes_received <= 0) {
    std::cerr << "Failed to receive data or client disconnected\n";
    closesocket(client);
    closesocket(server_fd);
    WSACleanup();
    return 1;
  } else 
  {
    buffer[bytes_received] = '\0'; // Null-terminate the received data

    // Extract the path parsing the string buffer

    std::string req = buffer;

    auto pos = req.find(' ');
    auto pos2 = req.find(' ', pos + 1);

    std::string path = req.substr(pos + 1, pos2 - pos - 1);

    // [8] Send the response back

    std::string response;

    if (path == "/") {
      response = "HTTP/1.1 200 OK\r\n\r\n";
    }
    else
    {
      response = "HTTP/1.1 404 Not Found\r\n\r\n";
    }

    send(client, response.c_str(), response.length(), 0);
  }

  // [9] Close connections
  
  closesocket(client);
  closesocket(server_fd);
  WSACleanup();

  return 0;
}
