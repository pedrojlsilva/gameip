#include <QCoreApplication>
#include <lib/myServer.h>
#include <stdio.h>
#include <string.h>

#define MSG_MAX_SIZE 350
#define BUFFER_SIZE (MSG_MAX_SIZE + 100)
#define LOGIN_MAX_SIZE 2
#define MAX_CHAT_CLIENTS 2
#define mensagem struct msg_ret_t


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
char aux[3];
int main()
{
    char client_names[MAX_CHAT_CLIENTS][LOGIN_MAX_SIZE];
    char str_buffer[BUFFER_SIZE], aux_buffer[BUFFER_SIZE], str_buffer2[1];
    serverInit(MAX_CHAT_CLIENTS);
    puts("Server is running!!");
    int x=1;
    while (1) {
      int id = acceptConnection();
      if (id != NO_CONNECTION) {
        recvMsgFromClient(client_names[id], id, WAIT_FOR_IT);
        str_buffer2[0] = id + '0';
        sendMsgToClient(str_buffer2, (int)strlen(str_buffer2) + 1 , id);
        printf("%s connected id = %d\n", client_names[id], id);
        x--;
      }
    }

    while(1){
        mensagem msgjog = recvMsg(aux_buffer);
        if (msgjog.status == MESSAGE_OK){
            printf("ola");
        }else if (msgjog.status == DISCONNECT_MSG){
            printf("desconectou");
        }
    }

}
