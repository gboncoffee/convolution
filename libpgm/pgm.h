#ifndef CONVOLUTION_PGM_H_
#define CONVOLUTION_PGM_H_

/*
 * Tries to adhere to the PGM spec:
 * https://netpbm.sourceforge.net/doc/pgm.html
 */

#include <stdint.h>
#include <stdlib.h>

typedef enum { P2, P5 } PGMType;

typedef struct {
    uint16_t *data;
    uint64_t width;
    uint64_t height;
    uint16_t maxVal;
} PGM;

/*
 * The lib assumes you're an adult and so it doesn't checks your pointer for
 * you. It'll be very sad if you pass it a null pointer.
 */

/* Propagates the error (errno). EDOM if the file is not a PGM. */
int ReadPGM(PGM *pgm, const char *filePath);
int WritePGM(PGM *pgm, const char *filePath, PGMType type);

void FreePGM(PGM *pgm);

/* Returns 1 on allocation error. */
int InitPGM(PGM *pgm, uint64_t width, uint64_t height, uint16_t maxVal);

/* Returns 1 if passing invalid row or column. Using signed integers here allow
 * for some INCREDIBLE optimizations. */
int SetPGMPixel(PGM *pgm, int64_t row, int64_t column, uint16_t pixel);
int GetPGMPixel(PGM *pgm, int64_t row, int64_t column, uint16_t *pixel);
int GetPGMPixelNormalized(PGM *pgm, int64_t row, int64_t column,
                          uint16_t *pixel);

int SetPGMPixelNormalized(PGM *pgm, int64_t row, int64_t column,
                          uint16_t pixel);

uint16_t GetPGMHeight(PGM *pgm);
uint16_t GetPGMWidth(PGM *pgm);
uint16_t GetPGMMaxVal(PGM *pgm);
void SetPGMMaxVal(PGM *pgm, uint16_t maxVal);
void NormalizePGMToNewMaxVal(PGM *pgm, uint16_t maxVal);

#define RENORMALIZE(x, oldMax, newMax) ((x * newMax) / oldMax)

#endif /* CONVOLUTION_PGM_H_ */
