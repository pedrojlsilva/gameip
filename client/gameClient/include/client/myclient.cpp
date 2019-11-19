#include "myclient.h"


MyClient::MyClient()
{
}
enum conn_ret_t MyClient::tryConnect(char server_ip[30]){
    return connectToServer(server_ip);
}

void MyClient::assertConnection(char server_ip[30]){
    enum conn_ret_t ans = this->tryConnect(server_ip);
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
        ans = tryConnect(server_ip);
      }
      char login[2];
      printf("Please enter your player (0 or 1): ", 2);
      scanf(" %[^\n]", login);
      getchar();
      int len = (int)strlen(login);
      this->sendMsg(login, len + 1);

}
int MyClient::sendMsg(void *type_buffer, int type_pointer){
    return 1; /*sendMsgToServer(type_buffer, type_pointer);*/
}
int MyClient::recvMsg(void *str_buffer){
    return 1; /*recvMsgFromServer(str_buffer, DONT_WAIT);*/
}
