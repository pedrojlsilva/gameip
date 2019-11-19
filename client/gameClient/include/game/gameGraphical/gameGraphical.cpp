#include "gameGraphical.h"

GameGraphical::GameGraphical(int width, int height, int fps, char *title){
    this->width = width;
    this->height = height;
    this->FPS = fps;
    this->title = title;

    setBallPosition(width/2, height/2);
    setBallRadius(raiobola);

    placar1 = 0;
    placar2 = 0;
    ballSpeed = 10;
    collision_count = 0;

    srand(time(NULL));

//    setBallMovement(randomMovement());
}

GameGraphical::~GameGraphical(){
    al_destroy_font(scoreBoard);
    al_destroy_display(mainWindow);
    al_destroy_event_queue(eventQueue);
}

bool GameGraphical::coreInit(){
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

bool GameGraphical::windowInit(){
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

bool GameGraphical::fontInit(int size){
    
    scoreBoard = al_load_font("resources/score.ttf", size, 0);

    if(!scoreBoard){
        fprintf(stderr, "Allegro couldn't load the font\n");
        return false;
    } 

    return true;
}

void GameGraphical::startTimer(){
    startingTime = al_get_time();
}

double GameGraphical::getTimer(){
    return (al_get_time() - startingTime);
}

void GameGraphical::FPSLimit(){
    if(getTimer() < 1.0/FPS){
        al_rest((1.0 / FPS) - getTimer());
    }
}

ALLEGRO_DISPLAY* GameGraphical::getWindow(){
    return mainWindow;
}

ALLEGRO_EVENT_QUEUE* GameGraphical::getEventQueue(){
    return eventQueue;
}

ALLEGRO_EVENT GameGraphical::getEvent(){
    return allegroEvent;
}

bool GameGraphical::isEventQueueEmpty(){
    return al_is_event_queue_empty(eventQueue);
}

void GameGraphical::waitForEvent(){
    al_wait_for_event(eventQueue, &allegroEvent);
}

void GameGraphical::flipAndClear(){
    al_flip_display();
    al_clear_to_color(al_map_rgb(0, 0, 0));
    FPSLimit();
}

void GameGraphical::drawMiddleLine(){
    int newTam = 0;
    for(int x = 1; x <= lineParts; x++){
        al_draw_line(width/2.0, newTam, width/2.0, x * (height/lineParts), al_map_rgb(255, 255, 255), 0);
        newTam = (x * (height/lineParts)) + spacingParts;
    }
}

Object GameGraphical::getBall(){
    return ball;
}

Object GameGraphical::getPlayer(int id){
    if(id >= qtPlayers || id < 0){
        fprintf(stderr, "Invalid object id\n");
        exit(-1);
    }

    return players[id];
}

void GameGraphical::drawBall(){
    al_draw_filled_circle(ball.x, ball.y, ballRadius, al_map_rgb(255, 255, 255));
}

void GameGraphical::drawPlayers(){
    for(int x = 0; x < qtPlayers; x++){
        al_draw_rectangle(players[x].x, players[x].y - tamPlayer, players[x].x, players[x].y + tamPlayer, al_map_rgb(255, 255, 255), 0);
    }
}

void GameGraphical::setBallPosition(int x, int y){
    this->ball.x = x;
    this->ball.y = y;
}

void GameGraphical::setPlayerPosition(int id, int x, int y){
    if(id < 0 || id >= qtPlayers){
        exit(-1);
    }
    players[id].x = x;
    players[id].y = y;
}

void GameGraphical::setBallRadius(double radius){
    ballRadius = radius;
}

void GameGraphical::drawScoreBoard(){
    char str1[3] = "";
    char str2[3] = "";

    sprintf(str1, "%d", placar1);
    sprintf(str2, "%d", placar2);

    al_draw_text(scoreBoard, al_map_rgb(255, 255, 255), width/2 - 20, 10, ALLEGRO_ALIGN_CENTER, str1);
    al_draw_text(scoreBoard, al_map_rgb(255, 255, 255), width/2 + 20, 10, ALLEGRO_ALIGN_CENTER, str2);
}

int GameGraphical::randomMovement(){
    return ((rand() % 6) + 3);
}
