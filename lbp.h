#ifndef CONVOLUTION_LBP_H_
#define CONVOLUTION_LBP_H_

#include "libpgm/pgm.h"

int GenerateLBPImage(char *inputFile, char *outputFile, PGM *outputImage);
/* Receives the output also to use as cache. */
int SearchBaseDirectory(char *baseDirectory, char *inputImage,
                        char *outputImage, PGM *lbpImage);

#endif /* CONVOLUTION_LBP_H_ */
