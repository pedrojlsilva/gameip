#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdbool.h>

#define MSG_MAX_SIZE 350
#define BUFFER_SIZE (MSG_MAX_SIZE + 100)
#define LOGIN_MAX_SIZE 13
#define HIST_MAX_SIZE 200

#define qtPlayers 2
#define lineParts 30
#define spacingParts 5

#define tamPlayer 50
#define playerMovement 20

#define collision_nheco 5

#define raiobola 5


#define WIDTH 640
#define HEIGHT 480
#define FramesPS 60.0
#define FONT_SIZE 32
char TITLE[] = "PongGame";

//--------------------Structs para o jogo------------
typedef struct{
    int x, y;
    int ogx, ogy;
    int movement;
} Object;

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

int recvData[9]; // armazena os dados de todos os jogadores, bola e Placar

	/*[0] = Pos X Player 1
	*	[1] = Pos Y Player 1
	*	[2] = Pos X Player 2
	*	[3] = Pos Y Player 2
	*	[4] = Pos X Bola
	*	[5] = Pos Y Bola
	*	[6] = Placar Player 1
  *	[7] = Placar Player 2
  *	[8] = Game Status

	*/
int sendData[2];
//--------------------Vars para o jogo------------

// ALLEGRO ITEMS
ALLEGRO_FONT *scoreBoard;
ALLEGRO_DISPLAY *mainWindow;
ALLEGRO_EVENT_QUEUE *eventQueue;
ALLEGRO_EVENT allegroEvent;
ALLEGRO_BITMAP *telaInicio = NULL;
ALLEGRO_BITMAP *telaPlayer1 = NULL;
ALLEGRO_BITMAP *telaPlayer2 = NULL;


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
// void setBallRadius(double radius);
void setPlayerMovement(int id, int movement);
void setBallMovement(int movement);
// void moveBall();
// getters
Object getPlayer(int id);
Object getBall();
int getPlayerMovement(int id);
int getBallMovement();
// collisions
bool checkCollision();
bool checkPlayerCollision(int id, int movement);

// random
int randomMovement();





//---------------------------Funções do server--------------------
void printHello();
enum conn_ret_t tryConnect();
void assertConnection();
void runChat();




int main() {
  assertConnection();
  
  construtorGame(WIDTH, HEIGHT, FramesPS, TITLE);
  if(!coreInit()) return -1;
  if(!windowInit()) return -1;
  if(!fontInit(FONT_SIZE)) return -1;
  telaInicio = al_load_bitmap("resources/newPlayer.png");
  telaPlayer1 = al_load_bitmap("resources/player1.png");
  telaPlayer2 = al_load_bitmap("resources/player2.png");

  for(int x = 0; x < qtPlayers; x++){
      setPlayerPosition(x, positions[x][0], positions[x][1]);
  }
  int run = true;
  
  while(run) {
    startTimer();
    while(!isEventQueueEmpty()){
            waitForEvent();
              if(getEvent().type == ALLEGRO_EVENT_KEY_DOWN ){ // key pressed
                    switch(getEvent().keyboard.keycode){
                        case ALLEGRO_KEY_UP:
                            sendData[1]=UP;
                            sendMsgToServer(sendData,(int)sizeof(sendData) + 1);
                            break;
                        case ALLEGRO_KEY_DOWN:
                            sendData[1]=DOWN;
                            sendMsgToServer(sendData,(int)sizeof(sendData) + 1);
                            break;
                    }
                } if(getEvent().type == ALLEGRO_EVENT_KEY_UP){
                    if(getEvent().keyboard.keycode == ALLEGRO_KEY_UP || getEvent().keyboard.keycode == ALLEGRO_KEY_DOWN){
                        sendData[1]=NO_MOVEMENT;
                        sendMsgToServer(sendData,(int)sizeof(sendData) + 1);

                    }
                    
                } if(getEvent().type == ALLEGRO_EVENT_DISPLAY_CLOSE){ // display closed
                    run = false;
                }
            

        }




    recvMsgFromServer(recvData, DONT_WAIT);
    
    fprintf(stdout, "mensagem recebida foi: %d %d \n ",  recvData[1],  recvData[3]);

    if(recvData[8]<2){
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_bitmap(telaInicio, 0, 0, 0);
        al_flip_display();

    }else if(recvData[8]==2){
        setBallPosition(recvData[4], recvData[5]);
        setPlayerPosition(0,recvData[0], recvData[1]);
        setPlayerPosition(1, recvData[2], recvData[3]);
        placar1=recvData[6];
        placar2=recvData[7];
        drawMiddleLine();
        drawScoreBoard();
        drawPlayers();
        drawBall();
        flipAndClear();
    }else if(recvData[8]==3){
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_bitmap(telaPlayer1, 0, 0, 0);
        al_flip_display();

    }else if(recvData[8]==4){
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_bitmap(telaPlayer2, 0, 0, 0);
        al_flip_display();
        
    }
    
  }
  return 0;
}




enum conn_ret_t tryConnect() {
  char server_ip[30];
  printf("Please enter the server IP: ");
  scanf(" %s", server_ip);
  getchar();
  return connectToServer(server_ip);
}

void printHello() {
  puts("Hello! Welcome to simple chat example.");
  puts("We need some infos to start up!");
}

void assertConnection() {
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
  printf("Please enter your login (0 or 1): ");
  scanf(" %[^\n]", login);
  getchar();
  int len = (int)strlen(login);
  sendMsgToServer(login, len + 1);

  recvMsgFromServer(&idPlayer,WAIT_FOR_IT); // recebe o id do jogador
  printf("your ID is: %d\n", idPlayer);
  sendData[0] = idPlayer-48;
}

void construtorGame(int _width, int _height, int _fps, char *_title){
    width = _width;
    height = _height;
    FPS = _fps;
    title = _title;
    ballRadius = raiobola;

    placar1 = 0;
    placar2 = 0;
    ballSpeed = 10;
    collision_count = 0;

}

void destructorGame(){
    al_destroy_font(scoreBoard);
    al_destroy_display(mainWindow);
    al_destroy_event_queue(eventQueue);
}

bool coreInit(){
    al_init_image_addon();

    if(!al_init()){
        fprintf(stderr, "Allegro couldn't be started\n");
        return false;
    }

    if(!al_init_image_addon()){
        fprintf(stderr, "Allegro couldn't init add-on allegro_image\n");
        return false;
    }

    if(!al_init_font_addon()){
        fprintf(stderr, "Allegro couldn't init add-on allegro_font\n");
        return false;
    }

    if(!al_init_ttf_addon()){
        fprintf(stderr, "Allegro couldn't init add-on allegro_ttf\n");
        return false;
    }

    if(!al_init_primitives_addon()){
        fprintf(stderr, "Allegro couldn't init add-on allegro_primitives\n");
        return false;
    }

    eventQueue = al_create_event_queue();
    if(!eventQueue){
        fprintf(stderr, "Allegro couldn't start event queue\n");
        return false;
    }

    if (!al_install_keyboard()){
        fprintf(stderr, "Falha ao inicializar teclado.\n");
        return false;
    }
    return true;
}

bool windowInit(){
    // window and event queue creation
    mainWindow = al_create_display(width, height);
    if(!mainWindow){
        fprintf(stderr, "Allegro couldn't create window\n");
        return false;
    }
    al_set_window_title(mainWindow, title);

    // register window on the event queue
    al_register_event_source(eventQueue, al_get_display_event_source(mainWindow));
    al_register_event_source(eventQueue, al_get_keyboard_event_source());

    return true;
}

bool fontInit(int size){
    
    scoreBoard = al_load_font("resources/score.ttf", size, 0);

    if(!scoreBoard){
        fprintf(stderr, "Allegro couldn't load the font\n");
        return false;
    } 

    return true;
}

void startTimer(){
    startingTime = al_get_time();
}

double getTimer(){
    return (al_get_time() - startingTime);
}

void FPSLimit(){
    if(getTimer() < 1.0/FPS){
        al_rest((1.0 / FPS) - getTimer());
    }
}

ALLEGRO_DISPLAY* getWindow(){
    return mainWindow;
}

ALLEGRO_EVENT_QUEUE* getEventQueue(){
    return eventQueue;
}

ALLEGRO_EVENT getEvent(){
    return allegroEvent;
}

bool isEventQueueEmpty(){
    return al_is_event_queue_empty(eventQueue);
}

void waitForEvent(){
    al_wait_for_event(eventQueue, &allegroEvent);
}

void flipAndClear(){
    al_flip_display();
    al_clear_to_color(al_map_rgb(0, 0, 0));
    FPSLimit();
}

void drawMiddleLine(){
    int newTam = 0;
    for(int x = 1; x <= lineParts; x++){
        al_draw_line(width/2.0, newTam, width/2.0, x * (height/lineParts), al_map_rgb(255, 255, 255), 0);
        newTam = (x * (height/lineParts)) + spacingParts;
    }
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

void drawBall(){
    al_draw_filled_circle(ball.x, ball.y, ballRadius, al_map_rgb(255, 255, 255));
}

void drawPlayers(){
    for(int x = 0; x < qtPlayers; x++){
        al_draw_rectangle(players[x].x, players[x].y - tamPlayer, players[x].x, players[x].y + tamPlayer, al_map_rgb(255, 255, 255), 0);
    }
}

void setBallPosition(int x, int y){
    ball.x = x;
    ball.y = y;
}

void setPlayerPosition(int id, int x, int y){
    players[id].x = x;
    players[id].y = y;
}



bool checkPlayerCollision(int id, int movement){
    if(movement == UP && getPlayer(id).y - tamPlayer >= 0) return false;
    if(movement == DOWN && getPlayer(id).y + tamPlayer <= height) return false;

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

void setPlayerMovement(int id, int movement){
    players[id].movement = movement;
}

void setBallMovement(int movement){
    ball.movement = movement;
}

int getPlayerMovement(int id){
    return players[id].movement;
}

int getBallMovement(){
    return ball.movement;
}




void drawScoreBoard(){
    char str1[3] = "";
    char str2[3] = "";

    sprintf(str1, "%d", placar1);
    sprintf(str2, "%d", placar2);

    al_draw_text(scoreBoard, al_map_rgb(255, 255, 255), width/2 - 20, 10, ALLEGRO_ALIGN_CENTER, str1);
    al_draw_text(scoreBoard, al_map_rgb(255, 255, 255), width/2 + 20, 10, ALLEGRO_ALIGN_CENTER, str2);
}

int randomMovement(){
    return ((rand() % 6) + 3);
}