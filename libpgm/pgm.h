#ifndef CONVOLUTION_PGM_H_
#define CONVOLUTION_PGM_H_

/*
 * Tries to adhere to the PGM spec:
 * https://netpbm.sourceforge.net/doc/pgm.html
 */

#include <stdint.h>

#define MAX_PGM_VALUE (UINT16_MAX)

typedef enum { P2, P5 } PGMType;

typedef struct {
    uint16_t *data;
    uint16_t width;
    uint16_t height;
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
int InitPGM(PGM *pgm, uint16_t width, uint16_t height);

/* Returns 1 if inexistent row, 2 if inexistent column. */
int GetPGMPixelNormalized(PGM *pgm, uint16_t row, uint16_t column,
                          uint16_t *pixel);

#endif /* CONVOLUTION_PGM_H_ */
