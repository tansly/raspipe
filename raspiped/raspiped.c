#include "globals.h"
#include <stdio.h>
#include <stdlib.h>


/* TODO: We will parse command line options,
 * or maybe even config files later on
 */
int main(void)
{
    int status = 0;
    status = start_server("0.0.0.0", "6666", 5, 5, 0);
    if (status != 0) {
        fprintf(stderr, "Failed to start server!\n");
        return 1;
    }
    cleanup();
    return 0;
}
