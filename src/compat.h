/*
 * compat.h — BSD to POSIX shims for Fenlason Hack restoration
 *
 * The original code targets V7 Unix on a PDP-11/70.
 * This header maps deprecated V7/BSD calls to modern equivalents.
 */

#ifndef COMPAT_H
#define COMPAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <stdarg.h>   /* Modern: for variadic functions (panic, pline, etc.) */
#include <termios.h>  /* Modern: replaces V7 <sgtty.h> */

/* V7 BSD string functions → POSIX */
#ifndef index
#define index strchr
#endif
#ifndef rindex
#define rindex strrchr
#endif

#endif /* COMPAT_H */
