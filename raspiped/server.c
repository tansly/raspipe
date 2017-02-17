#include "globals.h"
#include <assert.h>
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

/* TODO: Learn what happens if waitpid() returns -1.
 * May need a fix then.
 */
static void sigchld_handler(int sig)
{
    int old_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) { // Does not handle errors
        server.curr_clients--;
    }
    errno = old_errno;
}

static void terminate_handler(int sig)
{
    exit(cleanup());
}

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
        fprintf(stderr, "Could not bind anywhere!\n");
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

/* Main routine for fork()ed child
 * Currently just write()s what it recv()s to stdout
 * TODO: Error checking
 */
static void child_main(int recv_sock)
{
    char buf[BUFSIZ];
    size_t bufsiz = sizeof buf;
    ssize_t recved;
    /* Child will not need the signal handlers of the parent.
     * Do I really need to do this? Better check. TODO!!
     */
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = SIG_DFL;
    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // Now clean the server stuff, we do not need them
    free(server.bind_addr);
    free(server.bind_port);
    close(server.listen_sock);

    // We can now recv()! (I hope)
    // Stops on error or closed connection
    // Maybe treat errors seperately?
    while ((recved = recv(recv_sock, buf, bufsiz, 0)) > 0) {
        write(1, buf, recved);
    }
    close(recv_sock);
    exit(0);
}

/* TODO: Signal handlers: Now we handle SIGCHLD.
 * TODO: Daemonizing
 */
int start_server(const char *bind_addr, const char *bind_port,
                int max_clients, int backlog, int daemonize)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    server.curr_clients = 0;
    server.max_clients = max_clients;
    server.backlog = backlog;
    server.daemon = daemonize; // does nothing yet
    server.bind_addr = strdup(bind_addr);
    server.bind_port = strdup(bind_port);
    if (bind_and_listen() != 0) {
        free(server.bind_addr);
        free(server.bind_port);
        return 1;
    }
    sa.sa_handler = sigchld_handler;
    sigaction(SIGCHLD, &sa, NULL);
    sa.sa_handler = terminate_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    return 0; // YAY!!
}


/* TODO: 1) Error checking
 * 2) Get client address, log it or print it or something
 */
int main_loop(void)
{
    int recv_sock;
    pid_t pid;
    for (;;) {
        recv_sock = accept(server.listen_sock, NULL, NULL); // TODO 1) 2)
        if (recv_sock == -1) {
            // Currently we just go on to accept() another connection,
            // but we should properly handle different errors.
            perror("accept() error");
            continue;
        }
        fprintf(stderr, "Someone connected!\n");
        if (server.curr_clients >= server.max_clients) {
            fprintf(stderr, "Max number of clients (%d)"
                    "is reached.\n", server.max_clients);
            // This seems rude. Maybe notify the client?
            close(recv_sock);
            continue;
        }
        pid = fork();
        if (pid == -1) {
            fprintf(stderr, "Fork failed.\n");
            close(recv_sock);
        } else if (pid == 0) {
            child_main(recv_sock); // Will never return
            assert(0);
            exit(EXIT_FAILURE);
        } else {
            close(recv_sock);
            server.curr_clients++;
        }
    }
}

int cleanup(void)
{
    free(server.bind_addr);
    free(server.bind_port);
    close(server.listen_sock);
    return 0;
}
