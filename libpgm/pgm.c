#include "pgm.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int readPGMAsciiData(PGM *pgm, FILE *fp) {
    size_t size = pgm->width * pgm->height;
    size_t i;

    pgm->data = malloc(sizeof(uint16_t) * size);
    if (pgm->data == NULL) return errno;

    for (i = 0; i < size; i++) {
        if (fscanf(fp, "%hu", &pgm->data[i]) != 1) return errno;
    }

    return 0;
}

int readPGMBinaryData(PGM *pgm, FILE *fp) {
    size_t size = pgm->width * pgm->height;
    uint8_t pgmValue;
    size_t i;

    pgm->data = malloc(sizeof(uint16_t) * size);
    if (pgm->data == NULL) return errno;

    /* If is a 16 bit PGM we simply read. Else, it's 8 bit and we must convert
     * it.*/
    if (pgm->maxVal > UINT8_MAX) {
        if (fread(pgm->data, sizeof(uint16_t), size, fp) != size) return errno;

        return 0;
    }

    for (i = 0; i < size; i++) {
        if (fread(&pgmValue, 1, 1, fp) != 1) return errno;
        pgm->data[i] = (uint16_t)pgmValue;
    }

    return 0;
}

int readPGMMetadata(PGM *pgm, FILE *fp, PGMType type) {
    char c;
    /* Skip whitespace and comment lines as required by PGM specs. */
    do {
        c = getc(fp);
        switch (c) {
            case EOF:
                return EDOM;
                break;
            case '#':
                do {
                    c = getc(fp);
                    if (c == EOF) return EDOM;
                } while (c != '\n');
        }
    } while (c == ' ' || c == '\t' || c == '\n' || c == '\r');
    ungetc(c, fp);

    if (fscanf(fp, "%lu", &pgm->width) != 1) return errno;
    if (fscanf(fp, "%lu", &pgm->height) != 1) return errno;
    if (fscanf(fp, "%hu", &pgm->maxVal) != 1) return errno;

    if (pgm->maxVal == 0) return (errno = EDOM);

    /* "Single whitespace" tho I'm not bothering checking if it's a whitespace.
     * I just care if it's a single one. */
    if (getc(fp) == EOF) return errno;

    if (type == P2) return readPGMAsciiData(pgm, fp);
    return readPGMBinaryData(pgm, fp);
}

int ReadPGM(PGM *pgm, const char *filePath) {
    FILE *fp = fopen(filePath, "r");
    char c;
    int ret;
    if (fp == NULL) return errno;

    /* Check the first byte of the magic. */
    c = getc(fp);
    if (c == EOF) {
        int ret = errno;
        fclose(fp);
        return ret;
    } else if (c != 'P') {
        fclose(fp);
        return EDOM;
    }

    ret = 0;

    /* Check the second byte of the magic. */
    c = getc(fp);
    switch (c) {
        case '2':
            ret = readPGMMetadata(pgm, fp, P2);
            break;
        case '5':
            ret = readPGMMetadata(pgm, fp, P5);
            break;
        case EOF:
            ret = errno;
            break;
        default:
            ret = (errno = EDOM);
    }

    fclose(fp);
    return ret;
}

int writePGMBinary(PGM *pgm, FILE *fp) {
    size_t size = pgm->width * pgm->height;
    size_t i;

    fprintf(fp, "P5\n%lu %lu\n%hu\n", pgm->width, pgm->height, pgm->maxVal);

    /* Again we have to check if it should be a byte-size PGM or a word-size. */
    if (pgm->maxVal > UINT8_MAX) {
        if (fwrite(pgm->data, sizeof(uint16_t), size, fp) != size) return errno;

        return 0;
    }

    for (i = 0; i < size; i++) {
        if (fwrite(&pgm->data[i], 1, 1, fp) != 1) return errno;
    }

    return 0;
}

int writePGMAscii(PGM *pgm, FILE *fp) {
    size_t i, j;

    fprintf(fp, "P2\n%lu %lu\n%hu\n", pgm->width, pgm->height, pgm->maxVal);

    for (i = 0; i < pgm->height; i++) {
        /* I don't know why but the compiler complains about (pgm->width - 1)
         * beign signed. */
        for (j = 0; j < pgm->width - 1; j++)
            fprintf(fp, "%hu ", pgm->data[i * pgm->width + j]);
        fprintf(fp, "%hu\n", pgm->data[((i + 1) * pgm->width) - 1]);
    }

    return 0;
}

int WritePGM(PGM *pgm, const char *filePath, PGMType type) {
    int ret;

    FILE *fp = fopen(filePath, "w+");
    if (fp == NULL) return errno;

    ret = 0;
    if (type == P2)
        ret = writePGMAscii(pgm, fp);
    else
        ret = writePGMBinary(pgm, fp);

    fclose(fp);

    return ret;
}

int InitPGM(PGM *pgm, uint64_t width, uint64_t height, uint16_t maxVal) {
    pgm->height = height;
    pgm->width = width;
    pgm->maxVal = maxVal;

    pgm->data = calloc(width * height, sizeof(uint16_t));
    if (pgm->data == NULL) return errno;

    return 0;
}

void FreePGM(PGM *pgm) { free(pgm->data); }

int SetPGMPixel(PGM *pgm, int64_t row, int64_t column, uint16_t pixel) {
    if (pgm->height <= ((uint64_t)row) || pgm->width <= ((uint64_t)column) ||
        row < 0 || column < 0)
        return 1;
    pgm->data[row * pgm->width + column] = pixel;

    if (pixel > pgm->maxVal) pgm->maxVal = pixel;

    return 0;
}

uint16_t GetPGMHeight(PGM *pgm) { return pgm->height; }
uint16_t GetPGMWidth(PGM *pgm) { return pgm->width; }
uint16_t GetPGMMaxVal(PGM *pgm) { return pgm->maxVal; }
void SetPGMMaxVal(PGM *pgm, uint16_t maxVal) { pgm->maxVal = maxVal; }

int GetPGMPixel(PGM *pgm, int64_t row, int64_t column, uint16_t *pixel) {
    if (pgm->height <= ((uint64_t)row) || pgm->width <= ((uint64_t)column) ||
        row < 0 || column < 0)
        return 1;

    *pixel = pgm->data[row * pgm->width + column];

    return 0;
}

int GetPGMPixelNormalized(PGM *pgm, int64_t row, int64_t column,
                          uint16_t *pixel) {
    if (pgm->height <= ((uint64_t)row) || pgm->width <= ((uint64_t)column) ||
        row < 0 || column < 0)
        return 1;

    *pixel = RENORMALIZE(pgm->data[row * pgm->width + column], pgm->maxVal,
                         UINT16_MAX);

    return 0;
}

int SetPGMPixelNormalized(PGM *pgm, int64_t row, int64_t column,
                          uint16_t pixel) {
    if (pgm->height <= ((uint64_t)row) || pgm->width <= ((uint64_t)column) ||
        row < 0 || column < 0)
        return 1;

    pgm->data[row * pgm->width + column] =
        RENORMALIZE(pixel, UINT16_MAX, pgm->maxVal);

    return 0;
}

void NormalizePGMToNewMaxVal(PGM *pgm, uint16_t maxVal) {
    size_t size = pgm->width * pgm->height;
    size_t i;

    for (i = 0; i < size; i++)
        pgm->data[i] = RENORMALIZE(pgm->data[i], pgm->maxVal, maxVal);

    pgm->maxVal = maxVal;
}
