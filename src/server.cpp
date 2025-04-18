#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <winsock2.h>
#include <ws2tcpip.h>

int main(int argc, char **argv) {

  // Initialize Winsock
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

  // Uncomment this block to pass the first stage

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    WSACleanup();
    return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

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

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    closesocket(server_fd);
    WSACleanup();
    return 1;
  }

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  std::cout << "Waiting for a client to connect...\n";

  // Store the value returned by accept in a variable
  auto client = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);

  if (client == INVALID_SOCKET) {
    std::cerr << "accept failed\n";
    closesocket(server_fd);
    WSACleanup();
    return 1;
  }

  // Create a buffer to store the client's message
  char buffer[1024] = {0};
  char path[512] = {0};

  std::cout << "Client connected\n";

  int bytes_received = recv(client, buffer, sizeof(buffer), 0);

  // Extract the path 

  // strtok tokenizes the string buffer by the delimiter " " and returns a pointer to the first token (in this case the path)
  char* m_Path = strtok(buffer, " "); 
  m_Path = strtok(NULL, " ");

  // This will be used to store the number of bytes sent
  int bytes_sent;

  // Send the response back to the client according to the path
  if (strcmp(m_Path, "/") == 0) 
  {
    std::string message = "HTTP/1.1 200 OK\r\n\r\n";

    bytes_sent = send(client, message.c_str(), message.length(), 0);
  } 
  else
  {
    std::string message = "HTTP/1.1 404 Not Found\r\n\r\n";

    bytes_sent = send(client, message.c_str(), message.length(), 0);
  }

  if (bytes_sent < 0)
  {
    std::cerr << "Failed to send response\n";
    closesocket(client);
    closesocket(server_fd);
    WSACleanup();
    return 1;
  }

  close(client);
  close(server_fd);
  WSACleanup();

  return 0;
}
