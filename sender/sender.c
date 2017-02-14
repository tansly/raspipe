#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define HOSTNAME "raspberrypi.local"
#define PORT "6666"

/* TODO: Error checking
 * TODO: Get host from command line arguments
 * This is a prototype.
 * It will be improved once I figure things out.
 */
int main(void)
{
    int status;
    int sockfd;
    ssize_t read_bytes;
    struct addrinfo *servinfo, hints;
    char buffer[BUFSIZ];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(HOSTNAME, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo() error: %s\n", gai_strerror(status));
        return 1;
    }
    // TODO: Iterate over servinfo and try to socket() and connect()
    if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
        perror("socket() error");
        return 1;
    }
    if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        perror("connect() error");
        return 1;
    }
    freeaddrinfo(servinfo);
    // TODO: Proper error checking and handling for read() and send()
    while ((read_bytes = read(0, buffer, BUFSIZ)) > 0) {
        send(sockfd, buffer, read_bytes, 0);
    }
    close(sockfd);
    return 0;
}
