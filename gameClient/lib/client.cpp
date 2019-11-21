#include "client.h"
#include <poll.h>
#include <termios.h>
#include "default.h"

#define GETCH_TIMEOUT 100
#define READ_CONN_TIMEOUT 6
int network_socket;
fd_set sock_fd_set;

Client::Client()
{
}

void Client::closeConnection() {
  shutdown(network_socket, 3);
  close(network_socket);
}

/*
  Connects to server with IP informed as string on standard format
(X.X.X.X)
  If ServerIP is NULL connects to localhost (good to debug)
*/
enum conn_ret_t Client::connectToServer(const char *server_IP) {
  // create a socket for the client
  network_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (network_socket == -1) {
    perror("Could not create socket");
    exit(EXIT_FAILURE);
  }
  // making struct for server adress
  struct sockaddr_in server_address;
  memset(&server_address, 0, sizeof(server_address));  // clear struct
  server_address.sin_family = AF_INET;                 // set family
  server_address.sin_port = htons(PORT);               // working port
  if (server_IP == NULL) {  // if no IP sent, connect to localhost
    server_address.sin_addr.s_addr = INADDR_ANY;
  } else {
    server_address.sin_addr.s_addr = inet_addr(server_IP);
  }

  // Connect to server now
  int connection_status =
      connect(network_socket, (struct sockaddr *)&server_address,
              sizeof(server_address));
  enum conn_msg_t server_response = SUCCESSFUL_CONNECTION;

  FD_ZERO(&sock_fd_set);
  FD_SET(network_socket, &sock_fd_set);
  
  if (connection_status == 0) {
    // read server_response
    ssize_t conn_ans = recvMsgFromServer(&server_response, DONT_WAIT);
    unsigned int i = 0;
    while (i < READ_CONN_TIMEOUT && conn_ans == NO_MESSAGE) {
      i++;
      sleep(i);
      conn_ans = recvMsgFromServer(&server_response, DONT_WAIT);
    }
    if (conn_ans == SERVER_DISCONNECTED) {
      closeConnection();
      return SERVER_DOWN;
    } else if (conn_ans == NO_MESSAGE) {
      closeConnection();
      return SERVER_TIMEOUT;
    }
  } else if (connection_status == -1) {
    closeConnection();
    return SERVER_DOWN;
  }

  if (server_response == TOO_MANY_CLIENTS) {
    closeConnection();
    return SERVER_FULL;
  } else if (server_response == CONNECTIONS_CLOSED) {
    closeConnection();
    return SERVER_CLOSED;
  }

  FD_ZERO(&sock_fd_set);
  FD_SET(network_socket, &sock_fd_set);
  return SERVER_UP;
}

int Client::sendMsgToServer(void *msg, int size) {
  ssize_t size_ret = send(network_socket, &size, sizeof(int), MSG_NOSIGNAL);
  if (size_ret <= 0) {
    return SERVER_DISCONNECTED;
  }
  ssize_t total_size = 0;
  while (total_size < (ssize_t) size) {
    ssize_t msg_ret =
        send(network_socket, msg + total_size, (size_t)size, MSG_NOSIGNAL);
    if (msg_ret <= 0) {
      return SERVER_DISCONNECTED;
    }
    total_size += msg_ret;
  }
  return (int)total_size;
}

int Client::recvMsgFromServer(void *msg, int option) {
  if (option == DONT_WAIT) {
    struct timeval timeout = {0, SELECT_TIMEOUT};
    fd_set readfds = sock_fd_set;
    int sel_ret = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
    if (sel_ret < 0) {
      perror("select");
      exit(EXIT_FAILURE);
    }
    if (sel_ret == 0 || !FD_ISSET(network_socket, &readfds)) {  // timeout
      return NO_MESSAGE;
    }
  }
  int size;
  // get message size
  ssize_t size_ret = read(network_socket, &size, sizeof(int));
  if (size_ret <= 0) {
    return SERVER_DISCONNECTED;
  }
  // get message content
  ssize_t total_size = 0;
  while (total_size < (ssize_t) size) {
    ssize_t msg_ret = read(network_socket, msg + total_size, (size_t)size);
    if (msg_ret <= 0) {
      return SERVER_DISCONNECTED;
    }
    total_size += msg_ret;
  }
  return (int)total_size ;
}

/*
    modified getch();
    waits GETCH_TIMEOUT ms for key to be pressed, if it is not, returns
   NO_KEY_PRESSED
*/

char Client::getch() {
  struct termios oldt, newt;
  int ch;
  struct pollfd mypoll = {STDIN_FILENO, POLLIN | POLLPRI, 0};
  tcgetattr(STDIN_FILENO, &oldt);  // saving old config in oldt

  newt = oldt;
  newt.c_lflag &= ~((tcflag_t)(ICANON | ECHO));

  tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // setting new config

  if (poll(&mypoll, 1, GETCH_TIMEOUT)) {  // if key is pressed before timeout
    ch = getchar();
  } else {
    ch = NO_KEY_PRESSED;
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return (char)ch;
}

enum conn_ret_t Client::tryConnect() {
  char server_ip[30];
  printf("Please enter the server IP: ");
  scanf(" %s", server_ip);
  getchar();
  return this->connectToServer(server_ip);
}

void Client::printHello() {
  std::cout<<"Hello! Welcome to simple chat example."<<std::endl;
  std::cout<<"We need some infos to start up!"<<std::endl;
}

int Client::assertConnection() {
  this->printHello();
  enum conn_ret_t ans = tryConnect();
  while (ans != SERVER_UP) {
    if (ans == SERVER_DOWN) {
      puts("Server is down!");
    } else if (ans == SERVER_FULL) {
      puts("Server is full!");
    } else if (ans == SERVER_CLOSED) {
      puts("Server is closed for new connections!");
    } else {
      puts("Server didn't respond to connection!");
    }
    printf("Want to try again? [Y/n] ");
    int res;
    while (res = tolower(getchar()), res != 'n' && res != 'y' && res != '\n'){
      puts("anh???");
    }
    if (res == 'n') {
      exit(EXIT_SUCCESS);
    }
    ans = tryConnect();
  }
  char login[1];
  printf("Please enter your player (0 or 1): ");
  scanf(" %[^\n]", login);
  this->sendMsgToServer(login, sizeof (login)+1);
  char result[1];
  this->recvMsgFromServer(result, WAIT_FOR_IT);
  int rtn = atoi(result);
  return rtn;
}
