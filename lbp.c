#include "lbp.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include "libpgm/pgm.h"

void generateLBP(PGM *inputImage, PGM *outputImage) {
    uint16_t i, j;
    int16_t ni, nj;
    const uint16_t mask[3][3] = {{1, 2, 4}, {8, 0, 16}, {32, 64, 128}};
    uint16_t pixel;
    uint16_t neighbour;
    uint16_t result;

    /*
     * Note how it's more expensive to avoid computing the "lbp" for our pixel
     * then to just compute it but use a zeroed mask. So we do exactly that.
     *
     * No secret here: we just iterate each pixel, and iterate each of it's
     * neighbours. Set >=, multiply by the mask. Sum everything in result.
     */

    for (i = 0; i < inputImage->width; i++) {
        for (j = 0; j < outputImage->height; j++) {
            /* Guaranteed to not fail. */
            GetPGMPixel(inputImage, i, j, &pixel);
            result = 0;
            for (ni = -1; ni < 2; ni++) {
                for (nj = -1; nj < 2; nj++) {
                    neighbour = 0;
                    GetPGMPixel(inputImage, i + ni, j + nj, &neighbour);
                    neighbour = neighbour >= pixel ? 1 : 0;
                    neighbour *= mask[ni + 1][nj + 1];
                    result += neighbour;
                }
            }
            SetPGMPixel(outputImage, i, j, result);
        }
    }

    SetPGMMaxVal(outputImage, 255);
}

int GenerateLBPImage(char *inputFile, char *outputFile, PGM *outputImage) {
    PGM inputImage;

    if (ReadPGM(&inputImage, inputFile)) return errno;
    if (InitPGM(outputImage, inputImage.width, inputImage.height,
                inputImage.maxVal))
        return errno;

    generateLBP(&inputImage, outputImage);
    FreePGM(&inputImage);

    if (WritePGM(outputImage, outputFile, P5)) return errno;

    return 0;
}

int SearchBaseDirectory(char *baseDirectory, char *inputImage, char *outputImage, PGM *lbpImage) {
    struct dirent *ep;
    char *relPath;
    size_t relPathBufferSize;
    size_t entryPathSize;
    /* This strlen() should be safe as baseDirectory should be an argument. */
    size_t baseDirLen = strlen(baseDirectory);
    DIR *dp = opendir(baseDirectory);
    if (dp == NULL) return errno;

    /* To avoid too much allocations, let's allocate a string with an arbitrary
     * size and if needed we reallocate.*/
    relPath = malloc(baseDirLen + 256);
    if (relPath == NULL) return errno;
    relPathBufferSize = baseDirLen + 256;

    /* Again should be safe. */
    strcpy(relPath, baseDirectory);

    /* If our baseDirectory string doesn't have a / in the end, add it. */
    if (relPath[baseDirLen - 1] != '/') {
        relPath[baseDirLen] = '/';
        relPath[baseDirLen + 1] = '\0';
        baseDirLen++;
    }

    while ((ep = readdir(dp))) {
        entryPathSize = strlen(ep->d_name);
        if (entryPathSize + baseDirLen + 1 > relPathBufferSize) {
            relPath = realloc(relPath, entryPathSize + baseDirLen + 1);
            if (relPath == NULL) return errno;
        }
        strcpy(&relPath[baseDirLen + 1], ep->d_name);

        assert(0 && "TODO HERE");
    }
}
