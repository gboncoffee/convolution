#include <assert.h>
#include <stdio.h>

#include "libpgm/pgm.h"

int main(void) {
    PGM pgm;
    assert(ReadPGM(&pgm, "img/Apuleia1.pgm") == 0);
    assert(WritePGM(&pgm, "pgm-teste.pgm", P5));

    FreePGM(&pgm);

    return 0;
}
