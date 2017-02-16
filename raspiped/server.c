#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/* TODO: Better return some error codes on failure?
 */

static struct {
    int curr_clients;   // current number of clients 
    int max_clients;    // max number of clients, will not be much
    int backlog;
    int listen_sock;    // socket that the server is listening on
    int daemon;         // are we a daemon? not sure if needed
    char *bind_addr;    // address to bind
    char *bind_port;    // port to bind
} server;

/* Assuming that the server struct is filled.
 * Then bind and listen.
 * Set the server's listen_sock accordingly.
 * Return 0 on success, non-zero on error.
 */
static int bind_and_listen(void)
{
    int status;
    int sockfd;
    struct addrinfo *servinfo, *rp;
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    status = getaddrinfo(server.bind_addr, server.bind_port,
            &hints, &servinfo);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo() error: %s\n", gai_strerror(status));
        return 1;
    }
    for (rp = servinfo; rp; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) { // could not get socket
            continue; // try the next one
        }
        if (bind(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) { // we are done
            break;
        }
        close(sockfd); // could not bind, close and try the next one
    }
    freeaddrinfo(servinfo);
    if (!rp) { // no address succeeded
        return 1;
    }
    // Since bind() has succeeded, we now listen() the socket.
    status = listen(sockfd, server.backlog);
    if (status == -1) {
        perror("listen() error");
        return 1;
    }
    server.listen_sock = sockfd;
    return 0; // YAY
}

/* TODO: Signal handlers
 * TODO: Daemonizing
 */
int start_server(const char *bind_addr, const char *bind_port,
                int max_clients, int backlog, int daemonize)
{
    server.curr_clients = 0;
    server.max_clients = max_clients;
    server.backlog = backlog;
    server.daemon = daemonize; // does nothing yet
    server.bind_addr = strdup(bind_addr);
    server.bind_port = strdup(bind_port);
    if (bind_and_listen() != 0) {
        return 1;
    }
    return 0; // YAY!!
}

/* TODO: Do we need to wait() for children? */
int cleanup(void)
{
    free(server.bind_addr);
    free(server.bind_port);
    close(server.listen_sock);
    return 0;
}
