#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>


#include <vector>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
 #include <winsock2.h>
 #include <ws2tcpip.h>
 #define CLOSESOCKET closesocket
 #define GET_LAST_ERROR WSAGetLastError()
#else
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <unistd.h>
 #include <arpa/inet.h>
 #include <netdb.h>
 #include <sys/types.h>
 #define INVALID_SOCKET -1
 #define SOCKET_ERROR -1
 #define GET_LAST_ERROR errno
 #define CLOSESOCKET close
#endif

struct ParsedRequest
{
  std::string method;
  std::string path;
  std::string version;
  std::string headers;
  std::string body;
};

std::ostream& operator<<(std::ostream& os, const ParsedRequest& req)
{
  os << "Method: " << req.method << "\n";
  os << "Path: " << req.path << "\n";
  os << "Version: " << req.version << "\n";
  os << "Headers: " << req.headers << "\n";
  os << "Body: " << req.body << "\n";
  return os;
}

ParsedRequest parse_request(const std::string& request)
{
  ParsedRequest parsed_request;

  size_t pos = request.find(" ");
  parsed_request.method = request.substr(0, pos); // Extract method (e.g., GET, POST, PUT, etc...)

  size_t pos2 = request.find(" ", pos + 1);
  parsed_request.path = request.substr(pos + 1, pos2 - pos - 1); // Extract path (e.g., /echo/hello)

  size_t pos3 = request.find("\r\n", pos2 + 1);
  parsed_request.version = request.substr(pos2 + 1, pos3 - pos2 - 1); // Extract HTTP version (e.g., HTTP/1.1)

  size_t pos_headers = request.find("\r\n\r\n", pos3);
  parsed_request.headers = request.substr(pos3 + 2, pos_headers - pos3 - 2); // Extract headers (e.g., Host, User-Agent, etc...)

  parsed_request.body = request.substr(pos_headers + 4); // Extract body if present (e.g., data sent in POST request)

  return parsed_request;
}

std::vector<std::string> split(const std::string &path)
{
  std::vector<std::string> parts;

  size_t start = 0;

  while (start < path.size())
  {
    size_t next = path.find('/', start + 1); 

    // Search for all '/' characters in the string
    if (next == std::string::npos)
    {
      // Check if the remaining part of the string is not empty
      if (start + 1 < path.size())
      {
        parts.push_back(path.substr(start + 1, path.size() - start - 1)); // Extract the last part of the string
      }
      break;
    }

    parts.push_back(path.substr(start + 1, next - start - 1)); // Extract the part of the string between '/' characters

    start = next;
  }

  

  return parts;
}

// Function to create a response string
std::string make_response(int status, const std::string& reason, const std::string& body = "")
{
  std::ostringstream oss;

  oss << "HTTP/1.1 " << status << " " << reason << "\r\n"
  << "Content-Type: text/plain\r\n"
  << "Content-Length: " << body.size() << "\r\n"
  << "Connection: close\r\n"
  << "\r\n"
  << body;

  return oss.str();
}

// -----------------------------------------------------------------------------
// ---------------------------- Main function ----------------------------------
// -----------------------------------------------------------------------------

int main(int argc, char **argv) {

  // [1] Initialize Winsock

  #ifdef _WIN32

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
      std::cerr << "WSAStartup failed\n";
      return 1;
    }

  #endif

  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  // [2] Create a socket

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (server_fd == INVALID_SOCKET) {
    std::cerr << "Failed to create server socket\n";

    #ifdef _WIN32
      WSACleanup();
    #endif

    return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors

  // [3] Reuse the address

  int reuse = 1;

  #ifdef _WIN32
    // On Windows, we need to set the option on the socket itself
    if(setsockopt(server_fd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char*)&reuse, sizeof(reuse)) == SOCKET_ERROR) {
      std::cerr << "setsockopt failed\n";
      return 1;
    }
  #else
    // On Linux, we need to set the option on the socket itself
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) == SOCKET_ERROR) {
      std::cerr << "setsockopt failed\n";
      return 1;
    }
    
  #endif

  // [4] Bind the socket to the port

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {

    // The bind function returns SOCKET_ERROR on failure, not -1 like in Linux

    std::cerr << "Failed to bind to port 4221\n";
    CLOSESOCKET(server_fd);

    #ifdef _WIN32
      WSACleanup();
    #endif

    return 1;
  }

  // [5] Listen for incoming connections

  int connection_backlog = 5;

  if (listen(server_fd, connection_backlog) == SOCKET_ERROR) {
    std::cerr << "listen failed\n";
    CLOSESOCKET(server_fd);

    #ifdef _WIN32
      WSACleanup();
    #endif

    return 1;
  }

  std::cout << "Server listening on port 4221...\n";

  // [6] Accept incoming connections

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  // Store the value returned by accept in a variable
  #ifdef _WIN32
    SOCKET client = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
  #else
    int client = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *)&client_addr_len);
  #endif

  if (client == INVALID_SOCKET) {
    std::cerr << "accept failed\n";
    CLOSESOCKET(server_fd);

    #ifdef _WIN32
      WSACleanup();
    #endif

    return 1;
  }

  std::cout << "Client connected\n";

  // [7] Receive data from the client

  // Create a buffer to store the client's message
  char buffer[1024] = {0};

  recv(client, buffer, sizeof(buffer), 0);

  ParsedRequest parsed_request = parse_request(buffer);
  std::cout << "Received request:\n" << parsed_request << "\n";

  std::vector<std::string> parsed_path = split(parsed_request.path);

  if (parsed_request.path == "/")
  {
    std::string response = make_response(200, "OK");
    send(client, response.c_str(), response.size(), 0);
  }
  else if (parsed_path.size() == 2 && parsed_path[0] == "echo")
  {
    std::string response_body = parsed_path[1];
    std::string response = make_response(200, "OK", response_body);

    send(client, response.c_str(), response.size(), 0);
  }
  else
  {
    std::string response_notFound = make_response(404, "Not Found");
    send(client, response_notFound.c_str(), response_notFound.size(), 0);
  }

  // [9] Close connections
  
  CLOSESOCKET(client);
  CLOSESOCKET(server_fd);

  #ifdef _WIN32
      WSACleanup();
  #endif

  return 0;
}
