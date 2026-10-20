#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include "hack.h"

/* A separate module preserves the generator's process-local state and raw
 * level-file layout without merging its globals into the game's globals. */
EM_ASYNC_JS(int, generate_level, (const char *path, const char *kind, const char *depth), {
    try {
        const {default: createGenerator} = await import('./mklev.js');
        for (let attempt = 0; attempt < 100; ++attempt) {
            let status = 0;
            const generator = await createGenerator({
                arguments: ['level', UTF8ToString(kind), UTF8ToString(depth),
                            String(1 + Math.floor(Math.random() * 2147483646))],
                onExit: code => { status = code; },
                print: text => console.log('mklev:', text),
                printErr: text => console.error('mklev:', text)
            });
            if (status === 75) continue;
            if (status !== 0) return 0;
            FS.writeFile(UTF8ToString(path), generator.FS.readFile('level'));
            return 1;
        }
    } catch (error) {
        console.error('Level generation failed:', error);
    }
    return 0;
});

void wasm_mklev(const char *path, const char *kind, const char *depth)
{
    if (!generate_level(path, kind, depth)) {
        fputs("Cannot generate level. Reload to try again.\n", stderr);
        exit(1);
    }
}

/* Replace cat/the external pager with a terminal page every 20 lines. */
void wasm_page(const char *path, int paginate)
{
    FILE *fp = fopen(path, "r");
    int ch, lines = 0;
    if (!fp) return;
    cls();
    while ((ch = fgetc(fp)) != EOF) {
        putchar(ch);
        if (paginate && ch == '\n' && ++lines == 20) {
            getret();
            cls();
            lines = 0;
        }
    }
    fclose(fp);
    fflush(stdout);
}
