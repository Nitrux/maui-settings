#include "server.h"

int main(int argc, char *argv[])
{
    Server s(argc, argv);
//    s.setQuitOnLastWindowClosed(false);
    return s.exec();
}
