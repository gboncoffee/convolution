#ifndef CONVOLUTION_LBP_H_
#define CONVOLUTION_LBP_H_

#include "libpgm/pgm.h"

int GenerateLBPImage(char *inputFile, PGM *outputImage);
int GetImageVector(char *path, uint8_t *vector);
/* nearestFileName should be 256 positions long. */
int SearchBaseDirectory(char *baseDirectory, uint8_t *inputImageVector,
                        char *nearestFileName, double *distance);

#endif /* CONVOLUTION_LBP_H_ */
