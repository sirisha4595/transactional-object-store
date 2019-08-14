#include <getopt.h>
#include "debug.h"
#include "client_registry.h"
#include "transaction.h"
#include "protocol.h"
#include "store.h"
#include "csapp.h"
#include "server.h"
#include <sys/select.h>

static void terminate(int status);
int listenfd;
void handler(int sig)
{
    debug("received terminal line hangup:sig is:%d",sig);
    Close(listenfd);
    terminate(EXIT_SUCCESS);
    return;
}

CLIENT_REGISTRY *client_registry;

int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    Signal(SIGHUP,handler);
    extern char *optarg;
    extern int optind;
    int c, err = 0;
    //int hflag=0;
    int pflag=0;
    //int qflag=0;
    //char* infile;
    char* port;
    //char* hostname = "localhost";
    static char usage[] = "usage: %s [-h <host>] [-q] -p port\n";

    while ((c = getopt(argc, argv, "h:p:q")) != -1)
        switch (c) {
        case 'h':
            break;
        case 'p':
            if (pflag == 1) {
                    fprintf(stderr,"warning:  -p is set multiple times\n");
            }
            pflag = 1;
            debug("oparg is:%s",optarg);
            port = optarg;
            debug("got the port ");
            break;
        case 'q':
            break;
        case '?':
            err = 1;
            break;
        }
    if (pflag == 0) {   /* -p was mandatory */
        fprintf(stderr, "%s: missing -p option\n", argv[0]);
        fprintf(stderr, usage, argv[0]);
        exit(1);
    }
    else if (err) {
        fprintf(stderr, usage, argv[0]);
        exit(1);
    }
    // Perform required initializations of the client_registry,
    // transaction manager, and object store.
    client_registry = creg_init();
    debug("done with client_registry");
    trans_init();
    store_init();

    // Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function xacto_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.
    int *connfdp;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pthread_t tid;
    listenfd=Open_listenfd(port);
    while(1)
    {   debug("stanb by");
        connfdp=Malloc(sizeof(int));
        *connfdp= Accept(listenfd,(SA *) &clientaddr,&clientlen);
        Pthread_create(&tid,NULL,xacto_client_service,connfdp);
        debug("returned from thread routine");
    }

    fprintf(stderr, "You have to finish implementing main() "
	    "before the Xacto server will function.\n");

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    debug("calling shutdown");
    creg_shutdown_all(client_registry);
    debug("shutdown done");
    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    debug("creg fini");
    creg_fini(client_registry);
    debug("trans fini");
    trans_fini();
    store_fini();

    debug("Xacto server terminating with status:%d",status);
    exit(status);
}
