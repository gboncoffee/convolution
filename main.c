#include <assert.h>
#include <stdio.h>

#include "libpgm/pgm.h"

int main(void) {
    PGM pgm;
    assert(ReadPGM(&pgm, "pgm-teste.pgm") == 0);
    assert(WritePGM(&pgm, "pgm-teste-bin.pgm", P5) == 0);

    FreePGM(&pgm);

    return 0;
}
