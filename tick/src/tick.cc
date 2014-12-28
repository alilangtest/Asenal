#include "tick_server.h"
#include "xdebug.h"

#include <iostream>

tickConf *g_tickConf;

TickServer *g_tickServer;

void tick_signal_setup()
{
}

void tick_init_conf()
{

}


int main(int argc, char **argv)
{

    // set up the signal
    tick_signal_setup();

    // init the conf
    tick_init_conf();

    // Init the server
    g_tickServer = new TickServer();
    

    g_tickServer->RunProcess();

    return 0;
}
