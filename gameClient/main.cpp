#include <QCoreApplication>

#include <lib/client.h>
#include "include/game/gameGraphical/gameGraphical.h"


#define WIDTH 640
#define HEIGHT 480
#define FPS 60.0
#define FONT_SIZE 32
#define JOGADORES 2
char TITLE[] = "IP Game";

char dados[JOGADORES][3];
/*      [0] = armazena a posição x
	*	[1] = armazena a posição y
	*	[2] = armazena a tecla pressionada
	*/

GameGraphical pongGraphical = GameGraphical(WIDTH, HEIGHT, FPS, TITLE);
Client client = Client();

int main()
{
    dados[JOGADORES-1][0]='3';
    dados[JOGADORES-1][1]='2';
    dados[JOGADORES-1][2]='w';

    int rtn = client.assertConnection();
    std::cout<<"rtn assert is: "<< rtn <<std::endl;
    if(!pongGraphical.coreInit()){
        printf("Erro em InitCore");
        return -1;
    };
    if(!pongGraphical.windowInit()){
        printf("Erro em WindowInit");
        return -1;
    };
    if(!pongGraphical.fontInit(FONT_SIZE)){
        printf("Erro em fontInit");
        return -1;
    };

    while(1){
        pongGraphical.drawMiddleLine();
        pongGraphical.drawScoreBoard();
        pongGraphical.drawPlayers();
        pongGraphical.drawBall();
        pongGraphical.flipAndClear();
        client.sendMsgToServer(dados[1], 3);
    }
    pongGraphical.drawMiddleLine();
    
    

    return 0;
}
