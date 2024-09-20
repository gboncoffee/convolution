#include "lbp.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "libpgm/pgm.h"

int GenerateLBPImage(char *inputFile, char *outputFile, PGM *outputImage) {
    PGM inputImage;

    (void)outputFile;

    if (ReadPGM(&inputImage, inputFile)) return errno;
    if (InitPGM(outputImage, inputImage.width, inputImage.height,
                inputImage.maxVal))
        return errno;

    assert(0 && "TODO!!!!! NOW!!!!! THIS!!!!!");
    return 0;
}
