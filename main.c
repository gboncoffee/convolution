#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

/* ANSI C does not have a unistd.h with getopt. */
#include <getopt.h>

#include "lbp.h"
#include "libpgm/pgm.h"

int main(int argc, char *argv[]) {
    /* Parse args. */
    char *inputImage = NULL;
    char *outputImage = NULL;
    char *baseDirectory = NULL;
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
        ret = GenerateLBPImage(inputImage, outputImage, &lbpImage);
        if (ret != 0) return ret;

        ret = WritePGM(&lbpImage, outputImage, P5);
        if (ret != 0) return ret;
    }
    if (baseDirectory != NULL) assert(0 && "Not implemented.");

    return 0;
}
