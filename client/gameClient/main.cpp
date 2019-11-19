#include <QCoreApplication>

#include <include/client/myclient.h>
#include "include/game/gameGraphical/gameGraphical.h"


#define WIDTH 640
#define HEIGHT 480
#define FPS 60.0
#define FONT_SIZE 32
char TITLE[] = "IP Game";

//GameGraphical pongGraphical = GameGraphical(WIDTH, HEIGHT, FPS, TITLE);
Client client = Client();

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    int rtn = client.assertConnection();
    std::cout<<"rtn assert is: "<< rtn <<std::endl;

    return a.exec();
}
