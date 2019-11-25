#include "server.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdbool.h>

#define MSG_MAX_SIZE 350
#define BUFFER_SIZE (MSG_MAX_SIZE + 100)
#define LOGIN_MAX_SIZE 13
#define MAX_CHAT_CLIENTS 2
#define WIDTH 640
#define HEIGHT 480

#define FramesPS 60.0
#define FONT_SIZE 32
char TITLE[] = "PongGame";

#define qtPlayers 2
#define lineParts 30
#define spacingParts 5

#define tamPlayer 50
#define playerMovement 1

#define collision_nheco 5

#define raiobola 5


//--------------------Structs para o jogo------------

typedef enum{
    NO_MOVEMENT,
    UP,
    DOWN,
    LEFT,
    UPLEFT,
    DOWNLEFT,
    RIGHT,
    UPRIGHT,
    DOWNRIGHT
} Movements;

typedef struct{
    int x, y;
    int movement;
} Object;

int sendData[9]; // armazena os dados de todos os jogadores, bola e Placar

	/*[0] = Pos X Player 1
	*	[1] = Pos Y Player 1
	*	[2] = Pos X Player 2
	*	[3] = Pos Y Player 2
	*	[4] = Pos X Bola
	*	[5] = Pos Y Bola
	*	[6] = Placar Player 1
  *	[7] = Placar Player 2
  *	[8] = Status Game


	*/

int recvData[2];
//--------------------Vars para o jogo------------

// ALLEGRO ITEMS
ALLEGRO_FONT *scoreBoard;
ALLEGRO_DISPLAY *mainWindow;
ALLEGRO_EVENT_QUEUE *eventQueue;
ALLEGRO_EVENT allegroEvent;

// Window parameters
char *title;
int width;
int height;

// FPS Control
double startingTime;
double FPS;

// Objects
Object players[qtPlayers];
Object ball;
double ballRadius;

// Game parameters
int placar1;
int placar2;
int ballSpeed;
int collision_count;
int gameControl=0;

int idPlayer=0;

int positions[][2] = {
    {10, HEIGHT/2},
    {WIDTH - 10, HEIGHT/2}
};

//----------------------------Funções do jogo---------------------
void construtorGame(int _width, int _height, int _fps, char *_title);
void destructorGame();
bool coreInit();
bool windowInit();
bool fontInit(int size);

// getters
ALLEGRO_DISPLAY* getWindow();
ALLEGRO_EVENT_QUEUE* getEventQueue();
ALLEGRO_EVENT getEvent();

// allegro utils
void waitForEvent();
void flipAndClear();

// draw functions
void drawMiddleLine();

// checks
bool isEventQueueEmpty();

/* fps control */
void startTimer();
double getTimer();
void FPSLimit();

/* objects control */
// drawing
void drawPlayers();
void drawBall();
void drawScoreBoard();
// setters
void setBallPosition(int x, int y);
void setPlayerPosition(int id, int x, int y);
void setBallRadius(double radius);

void setBallMovement(int movement);
void moveBall();
// getters
Object getPlayer(int id);
Object getBall();
int getBallMovement();
// collisions
bool checkCollision();
bool checkPlayerCollision(int id, int movement);
// goal
void checkGoal();
void addScore(int id);
// random
int randomMovement();
void mountPack(int gameStatus);




int main() {

  char client_names[MAX_CHAT_CLIENTS][LOGIN_MAX_SIZE];
  char str_buffer2[2];
  serverInit(MAX_CHAT_CLIENTS);
  puts("Server is running!!");

  construtorGame(WIDTH, HEIGHT, FramesPS, TITLE);
  

  while (1) {
    startTimer();

    int id = acceptConnection();
    if (id != NO_CONNECTION) {
      recvMsgFromClient(client_names[id], id, WAIT_FOR_IT);
      str_buffer2[0] = id + '0';
      sendMsgToClient(str_buffer2, (int)strlen(str_buffer2) + 1 , id);
      gameControl++;
    }
    

    


    // if (msg_ret.status == DISCONNECT_MSG){
    //     gameControl=0;
    //   printf("%s disconnected, id = %d is free gameControl is %d\n",
    //          client_names[msg_ret.client_id], msg_ret.client_id, gameControl);
        
    //     mountPack(gameControl);
    //     broadcast(sendData, (int)sizeof(sendData) + 1);

    // }
    if(gameControl==2){
        struct msg_ret_t msg_ret = recvMsg(recvData);
        int infoRec=recvData[1];
        int idRec=recvData[0];

        if(idRec==0){
        if(infoRec == 1){
            if(!checkPlayerCollision(0, 1)){
                setPlayerPosition(0, positions[0][0], players[0].y - playerMovement);
            }
        }else if(infoRec == 2){
            if(!checkPlayerCollision(0, 2)){
                setPlayerPosition(0, positions[0][0], players[0].y + playerMovement); 
            } 
        }
        
            
        }

        if (idRec == 1){
            if(infoRec == 1){
                if(!checkPlayerCollision(1, 1)){
                    setPlayerPosition(1, positions[1][0], players[1].y - playerMovement); 
                }
            }else if (infoRec == 2){
                if(!checkPlayerCollision(1, 2)){
                    
                    setPlayerPosition(1, positions[1][0], players[1].y + playerMovement); 
                }
            }
        }
        moveBall();
        
        sendData[0]=players[0].x;
        sendData[1]=players[0].y;
        sendData[2]=players[1].x;
        sendData[3]=players[1].y;
        sendData[4]=ball.x;
        sendData[5]=ball.y;
        sendData[6]=placar1;
        sendData[7]=placar2;
        sendData[8]=gameControl;
        printf("%d\n", sendData[3]);
        broadcast(sendData, (int)sizeof(sendData) + 1);
    }
    // }else if(gameControl==3){
    //     mountPack(gameControl);
    //     broadcast(sendData, (int)sizeof(sendData) + 1);

    // }else if(gameControl==4){
    //     mountPack(gameControl);
    //     broadcast(sendData, (int)sizeof(sendData) + 1);
    // }
    
  }
  
}



void construtorGame(int _width, int _height, int _fps, char *_title){
    width = _width;
    height = _height;
    FPS = _fps;
    title = _title;

    setBallPosition(width/2, height/2);
    setBallRadius(raiobola);

    placar1 = 0;
    placar2 = 0;
    ballSpeed = 10;
    collision_count = 0;
    for(int x = 0; x < qtPlayers; x++){
        setPlayerPosition(x, positions[x][0], positions[x][1]);
    }

    srand(time(NULL));

    setBallMovement(randomMovement());
}







void startTimer(){
    startingTime = al_get_time();
}

double getTimer(){
    return (al_get_time() - startingTime);
}



bool isEventQueueEmpty(){
    return al_is_event_queue_empty(eventQueue);
}

void waitForEvent(){
    al_wait_for_event(eventQueue, &allegroEvent);
}


Object getBall(){
    return ball;
}

Object getPlayer(int id){
    if(id >= qtPlayers || id < 0){
        fprintf(stderr, "Invalid object id\n");
        exit(-1);
    }

    return players[id];
}


void setBallPosition(int x, int y){
    ball.x = x;
    ball.y = y;
}

void setPlayerPosition(int id, int x, int y){
    if(id < 0 || id >= qtPlayers){
        exit(-1);
    }

    players[id].x = x;
    players[id].y = y;
}

void setBallRadius(double radius){
    ballRadius = radius;
}

void checkGoal(){
    if((ball.x + ballRadius <= 0)){
        addScore(1); // player 2 goal
    }else if(ball.x + ballRadius >= width){
        addScore(0); // player 1 goal
    }
}

bool checkPlayerCollision(int id, int movement){
    if(movement == 1 && getPlayer(id).y - tamPlayer >= 0) return false;
    if(movement == 2 && getPlayer(id).y + tamPlayer <= height) return false;

    return true;
}

bool checkCollision(){
    int rand = 0;

    if((ball.y - ballRadius) <= 0){ // top corner
        if(getBallMovement() == UPLEFT){
            rand = DOWNLEFT;
        }else if(getBallMovement() == UPRIGHT){
            rand = DOWNRIGHT;
        }
        setBallMovement(rand);
        return true;
    }else if((ball.y + ballRadius) >= height){ // bottom corner
        if(getBallMovement() == DOWNLEFT){
            rand = UPLEFT;
        }else if(getBallMovement() == DOWNRIGHT){
            rand = UPRIGHT;
        }
        setBallMovement(rand);
        return true;
    }
    if((ball.x - ballRadius <= players[0].x) && (ball.y - ballRadius <= (players[0].y + tamPlayer)) && (ball.y + ballRadius >= (players[0].y - tamPlayer))){
        while(rand == NO_MOVEMENT || rand == LEFT || rand == UPLEFT){
            rand = randomMovement();
        }
        setBallMovement(rand);
        return true;
    }
    if((ball.x + ballRadius >= players[1].x) && (ball.y - ballRadius <= (players[1].y + tamPlayer)) && (ball.y + ballRadius >= (players[1].y - tamPlayer))){
        while(rand == NO_MOVEMENT || rand == RIGHT || rand == UPRIGHT){
            rand = randomMovement();
        }
        setBallMovement(rand);
        return true;
    }
    // to do: colision with player
}




void setBallMovement(int movement){
    ball.movement = movement;
}


int getBallMovement(){
    return ball.movement;
}


void moveBall(){
    if(ball.movement == LEFT){
        ball.x -= ballSpeed;
    }else if(ball.movement == RIGHT){
        ball.x += ballSpeed;
    }else if(ball.movement == UPLEFT){
        ball.x -= ballSpeed/2;
        ball.y -= ballSpeed/2;
    }else if(ball.movement == DOWNLEFT){
        ball.x -= ballSpeed/2;
        ball.y += ballSpeed/2;
    }else if(ball.movement == UPRIGHT){
        ball.x += ballSpeed/2;
        ball.y -= ballSpeed/2;
    }else if(ball.movement == DOWNRIGHT){
        ball.x += ballSpeed/2;
        ball.y += ballSpeed/2;
    }else if(ball.movement == NO_MOVEMENT){}
    
    if(!checkCollision()){
        checkGoal();
    }else{
        collision_count++;
        if(collision_count == 5){
            collision_count = 0;
            ballSpeed +=4;
        }
    }
}

void addScore(int id){
    if(id == 0) placar1++;
    else if(id == 1) placar2++;

    // if(placar1==10){
    //     gameControl=3;
    // }
    // if(placar2==10){
    //     gameControl=4;
    // }


    setBallPosition(width/2, height/2);
    setBallMovement(randomMovement());
    ballSpeed = 10;
    collision_count = 0;
}

int randomMovement(){
    return ((rand() % 6) + 3);
}

void mountPack(int gameStatus){
    sendData[0]=players[0].x;
    sendData[1]=players[0].y;
    sendData[2]=players[1].x;
    sendData[3]=players[1].y;
    sendData[4]=ball.x;
    sendData[5]=ball.y;
    sendData[6]=placar1;
    sendData[7]=placar2;
    sendData[8]=gameStatus;

}