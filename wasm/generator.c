#include <stdlib.h>
#include <unistd.h>

/* mklev re-execs itself when corridor generation fails. Let the caller
 * retry with a fresh module instead; no original generator edits needed. */
int execl(const char *path, const char *arg, ...)
{
    (void)path;
    (void)arg;
    exit(75);
}
