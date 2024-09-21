#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/* ANSI C does not have a unistd.h with getopt. */
#include <getopt.h>

#include "lbp.h"
#include "libpgm/pgm.h"

int main(int argc, char *argv[]) {
    /* Parse args. */
    char *inputImage = NULL;
    char *outputImage = NULL;
    char *baseDirectory = NULL;
    char *nearestFileName;
    double nearestDistance;
    uint8_t *inputImageVector;
    PGM lbpImage;
    int ret;

    char nextOpt;
    while ((nextOpt = getopt(argc, argv, "i:o:d:")) != -1) {
        switch (nextOpt) {
            case 'i':
                inputImage = optarg;
                break;
            case 'o':
                outputImage = optarg;
                break;
            case 'd':
                baseDirectory = optarg;
                break;
        }
    }

    if (inputImage == NULL || (outputImage == NULL && baseDirectory == NULL))
        return EDOM;
    if (outputImage != NULL) {
        ret = GenerateLBPImage(inputImage, &lbpImage);
        if (ret != 0) return ret;

        if (WritePGM(&lbpImage, outputImage, P5)) return errno;

        ret = WritePGM(&lbpImage, outputImage, P5);
        FreePGM(&lbpImage);
        if (ret != 0) return ret;
    }
    if (baseDirectory != NULL) {
        inputImageVector = malloc(256);
        if (inputImageVector == NULL) return errno;

        ret = GetImageVector(inputImage, inputImageVector);
        if (ret != 0) {
            free(inputImageVector);
            return ret;
        }
        ret = SearchBaseDirectory(baseDirectory, inputImageVector,
                                  &nearestFileName, &nearestDistance);
        if (ret != 0) {
            free(inputImageVector);
            return ret;
        }

        printf("Imagem mais similar: %s %.6f\n", nearestFileName,
               nearestDistance);
        free(inputImageVector);
        free(nearestFileName);
    }

    return 0;
}
