#ifndef CONVOLUTION_LBP_H_
#define CONVOLUTION_LBP_H_

#include "libpgm/pgm.h"

int GenerateLBPImage(char *inputFile, PGM *outputImage);
int GetImageVector(char *path, uint8_t *vector);
/* Receives the output also to use as cache. */
int SearchBaseDirectory(char *baseDirectory, uint8_t *inputImageVector,
                        char **fileName, double *distance);

#endif /* CONVOLUTION_LBP_H_ */
