#include "pgm.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int readPGMAsciiData(PGM *pgm, FILE *fp) {
    (void)pgm;
    (void)fp;
    assert(0 && "Not implemented.");
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
    /* Skip whitespace as required by PGM specs. */
    do {
        c = getc(fp);
        if (c == EOF) return EDOM;
    } while (c == ' ' || c == '\t' || c == '\n' || c == '\r');
    ungetc(c, fp);

    if (fscanf(fp, "%hu", &pgm->width) != 1) return errno;
    if (fscanf(fp, "%hu", &pgm->height) != 1) return errno;
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

    fprintf(fp, "P5\n%hd %hd\n%hd\n", pgm->width, pgm->height, pgm->maxVal);

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
    (void)pgm;
    (void)fp;
    assert(0 && "Not implemented.");
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

    return ret;
}

int InitPGM(PGM *pgm, uint16_t width, uint16_t height) {
    pgm->height = height;
    pgm->width = width;

    pgm->data = malloc(sizeof(uint16_t) * width * height);
    if (pgm->data == NULL) return errno;

    return 0;
}

void FreePGM(PGM *pgm) { free(pgm->data); }
