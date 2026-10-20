#ifndef PROTOHACK_WASM_H
#define PROTOHACK_WASM_H
/* Browser-only process replacements; injected through compat.h. */
char *wasm_getlogin(void);
#define getlogin wasm_getlogin
void wasm_mklev(const char *, const char *, const char *);
void wasm_page(const char *, int);
#endif
