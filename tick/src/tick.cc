#include "tick_server.h"
#include "xdebug.h"
#include <glog/logging.h>
#include "tick_conf.h"


#include <iostream>
#include <signal.h>

TickConf *g_tickConf;

TickServer *g_tickServer;


static void tick_glog_init()
{
    log_info("%s", g_tickConf->log_path());
    FLAGS_log_dir = g_tickConf->log_path();
    google::InitGoogleLogging("tick");
    LOG(INFO) << "tick glog init ok";
    /*
     * dump some usefull message when crash on certain signals
     */
    google::InstallFailureSignalHandler();
}

static void sig_handler(const int sig)
{
    printf("Caught signal %d", sig);
}

void tick_signal_setup()
{
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    LOG(INFO) << "tick signal setup ok";
}

void tick_init_conf(const char* path)
{
    g_tickConf = new TickConf(path);
    if (g_tickConf == NULL) {
        LOG(FATAL) << "tick load conf error";
    }
    g_tickConf->DumpConf();
}

static void usage() {
    fprintf(stderr,
            "Tick module 1.0.0\n"
            "need One parameters\n"
            "-D the conf path \n"
            "-h show this usage message \n"
            "example: ./tick -D./conf/tick.conf\n"
           );
}

int main(int argc, char **argv)
{
    bool path_opt = false;
    char c;
    char path[TICK_LINE_SIZE];
    if (argc != 2) {
        usage();
        return -1;
    }
    while (-1 != (c = getopt(argc, argv, "D:h"))) {
        switch (c) {
        case 'D':
            snprintf(path, TICK_LINE_SIZE, "%s", optarg);
            path_opt = 1;
            break;
        case 'h':
            usage();
            return 0;
        default:
            usage();
            return 0;
        }
    }

    /*
     * check wether set the conf path
     */
    if (path_opt == false) {
        LOG(FATAL) << "Don't get the conf path";
    }

    /*
     * init the conf
     */
    tick_init_conf(path);

    /*
     * init the glog config
     */
    tick_glog_init();

    /*
     * set up the signal
     */
    tick_signal_setup();

    /*
     * Init the server
     */
    g_tickServer = new TickServer();
    
    if (g_tickServer != NULL) {
        LOG(INFO) << "Tick Server init ok";
    }


    LOG(INFO) << "Tick Server going to start";
    g_tickServer->RunProcess();

    return 0;
}
