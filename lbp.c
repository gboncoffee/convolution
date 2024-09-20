#include "lbp.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>

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

    (void)outputFile;

    if (ReadPGM(&inputImage, inputFile)) return errno;
    if (InitPGM(outputImage, inputImage.width, inputImage.height,
                inputImage.maxVal))
        return errno;

    generateLBP(&inputImage, outputImage);

    if (WritePGM(outputImage, outputFile, P5)) return errno;

    return 0;
}
