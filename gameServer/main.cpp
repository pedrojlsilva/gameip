#include <QCoreApplication>
#include <lib/myServer.h>
#include <stdio.h>
#include <string.h>

#define MSG_MAX_SIZE 350
#define BUFFER_SIZE (MSG_MAX_SIZE + 100)
#define LOGIN_MAX_SIZE 2
#define MAX_CHAT_CLIENTS 2



typedef struct{
    int x, y;
    int ogx, ogy;
} Object;

typedef struct
{
  int player1, player2;


}placar;

typedef struct
{
  placar placarJogo;
  Object ball, player1, player2;

}gameBackbone;

int main()
{
    char client_names[MAX_CHAT_CLIENTS][LOGIN_MAX_SIZE];
    char str_buffer[BUFFER_SIZE], aux_buffer[BUFFER_SIZE];
    serverInit(MAX_CHAT_CLIENTS);
    puts("Server is running!!");
    while (1) {
      int id = acceptConnection();
      if (id != NO_CONNECTION) {
        recvMsgFromClient(client_names[id], id, WAIT_FOR_IT);
        sendMsgToClient(&id, sizeof(int), id);
        printf("%s connected id = %d\n", client_names[id], id);
      }

//      gameBackbone game = recvMsg(aux_buffer);
//      if (msg_ret.status == MESSAGE_OK) {
//        sprintf(str_buffer, "%s-%d: %s", client_names[msg_ret.client_id],
//                msg_ret.client_id, aux_buffer);
//        broadcast(str_buffer, (int)strlen(str_buffer) + 1);
//      } else if (msg_ret.status == DISCONNECT_MSG) {
//        sprintf(str_buffer, "%s disconnected", client_names[msg_ret.client_id]);
//        printf("%s disconnected, id = %d is free\n",
//               client_names[msg_ret.client_id], msg_ret.client_id);
//        broadcast(str_buffer, (int)strlen(str_buffer) + 1);
//      }
    }
}
