#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* Stable save/score identity. Randomness must not depend on Emscripten's
 * constant process ID, as the native game and generator both seed with it. */
char *wasm_getlogin(void) { return "player"; }
pid_t getpid(void) { return (pid_t)((unsigned)time(NULL) & 0x7fffffff); }

/* Every module has a private MEMFS; there are no other processes sharing
 * its lock files. Avoid libc's process stubs (and their passwd symbols). */
int flock(int fd, int operation) { (void)fd; (void)operation; return 0; }
