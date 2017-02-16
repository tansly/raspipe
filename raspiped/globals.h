/* This header contains the declarations of functions
 * that will be shared between source files.
 */
#ifndef __GLOBALS_H__
#define __GLOBALS_H__

/* SERVER FUNCTIONS */

/* Initialize everything and start listening for connections.
 * Returns 0 on success, non-zero on error
 */
int start_server(const char *bind_addr, const char *bind_port,
                int max_clients, int backlog, int daemonize);

/* Do the necessary cleanup and shut the server down.
 * Returns 0 on success, non-zero on error
 */
int cleanup(void);

/* The main loop of the program.
 * accept() the connections, recv() data,
 * and?
 */
int main_loop(void);

//////////////////////////////////////////////////////////////////////

#endif
