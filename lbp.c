#include "lbp.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libpgm/pgm.h"

void generateLBP(PGM *inputImage, PGM *outputImage) {
    uint64_t i, j;
    int64_t ni, nj;
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

    for (i = 0; i < GetPGMHeight(outputImage); i++) {
        for (j = 0; j < GetPGMWidth(outputImage); j++) {
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

int GenerateLBPImage(char *inputFile, PGM *outputImage) {
    PGM inputImage;

    if (ReadPGM(&inputImage, inputFile)) return errno;
    if (InitPGM(outputImage, GetPGMWidth(&inputImage),
                GetPGMHeight(&inputImage), GetPGMMaxVal(&inputImage)))
        return errno;

    generateLBP(&inputImage, outputImage);
    FreePGM(&inputImage);

    return 0;
}

int computeImageVector(char *path, uint8_t *vector) {
    PGM image;
    size_t i, j;
    uint16_t pixel;
    if (GenerateLBPImage(path, &image)) return errno;

    memset(vector, 0, 256);

    for (i = 0; i < GetPGMHeight(&image); i++) {
        for (j = 0; j < GetPGMWidth(&image); j++) {
            GetPGMPixel(&image, i, j, &pixel);
            vector[(uint8_t)pixel]++;
        }
    }

    return 0;
}

int makeSamePathWithLBPExtension(char *path, char **newPath) {
    size_t pathLen = strlen(path);
    int64_t dotPosition;
    for (dotPosition = pathLen - 1;
         dotPosition >= 0 && path[dotPosition] != '.'; dotPosition--)
        ;
    if (dotPosition < 0) dotPosition = pathLen;
    *newPath = malloc(dotPosition + 5);
    if (*newPath == NULL) return errno;

    strncpy(*newPath, path, dotPosition);
    (*newPath)[dotPosition] = '\0';
    strcat(*newPath, ".lbp");
    return 0;
}

int saveVectorFile(char *lbpPath, uint8_t *vector) {
    int ret;
    FILE *fp = fopen(lbpPath, "w");
    if (fp == NULL) return errno;

    ret = fwrite(vector, 1, 256, fp) == 256 ? 0 : errno;

    fclose(fp);
    return ret;
}

int GetImageVector(char *path, uint8_t *vector) {
    char *lbpPath = NULL;
    FILE *fp;
    if (makeSamePathWithLBPExtension(path, &lbpPath) > 0) return errno;

    fp = fopen(lbpPath, "r");
    if (fp == NULL) {
        /* This error should simply be ignored because we may be trying to
         * search a file or something. */
        if (computeImageVector(path, vector))
            return -1;
        return saveVectorFile(lbpPath, vector);
    }
    if (fread(vector, 1, 256, fp) != 256) {
        fclose(fp);
        /* Same. */
        if (computeImageVector(path, vector))
            return -1;
        return saveVectorFile(lbpPath, vector);
    }

    free(lbpPath);
    return 0;
}

int getDistance(char *path, uint8_t *baseVector, double *newDistance) {
    size_t i;
    int64_t distance;
    int64_t cur;
    int ret;
    uint8_t *newVector = malloc(256);
    if (newVector == NULL) return errno;

    /* If GetImageVector returns EDOM, it means the path is not a PGM.*/
    ret = GetImageVector(path, newVector);
    if (ret == EDOM || ret < 0) {
        errno = 0;
        return -1;
    } else if (ret != 0) {
        return errno;
    }

    /* Computes the distance. */
    distance = 0;
    for (i = 0; i < 256; i++) {
        cur = baseVector[i] - newVector[i];
        cur *= cur;
        distance += cur;
    }

    *newDistance = sqrt((double) distance);

    free(newVector);
    return 0;
}

char *makeRelPathBuffer(char *baseDirectory, size_t *relPathBufferSize,
                        size_t *baseDirLen) {
    char *relPath;
    /* This strlen() should be safe as baseDirectory should be an argument. */
    *baseDirLen = strlen(baseDirectory);
    relPath = malloc(*baseDirLen + 256);
    if (relPath == NULL) return NULL;
    *relPathBufferSize = *baseDirLen + 256;

    /* Again should be safe. */
    strcpy(relPath, baseDirectory);

    /* If our baseDirectory string doesn't have a / in the end, add it. */
    if (relPath[*baseDirLen - 1] != '/') {
        relPath[*baseDirLen] = '/';
        relPath[*baseDirLen + 1] = '\0';
        (*baseDirLen)++;
    }

    return relPath;
}

int SearchBaseDirectory(char *baseDirectory, uint8_t *inputImageVector, char **fileName, double *distance) {
    struct dirent *ep;
    char *relPath;
    char *nearestFileName;
    double nearestDistance;
    double newDistance;
    size_t relPathBufferSize;
    size_t entryPathSize;
    size_t baseDirLen;
    size_t nearestFileNameSize;

    DIR *dp = opendir(baseDirectory);
    if (dp == NULL) return errno;

    /* To avoid too much allocations, let's allocate a string with an arbitrary
     * size and if needed we reallocate.*/
    relPath = makeRelPathBuffer(baseDirectory, &relPathBufferSize, &baseDirLen);
    if (relPath == NULL) goto cleanup_dp;

    /* We'll be doing the same with the nearestFileName buffer. */
    nearestFileName = calloc(relPathBufferSize, 1);
    if (nearestFileName == NULL) goto cleanup_rel_path;
    nearestFileNameSize = relPathBufferSize;

    /* Init the distance with HUGE_VAL for obvious reasons. */
    nearestDistance = HUGE_VAL;

    /* Iterate checking the distance. */
    while ((ep = readdir(dp))) {
        entryPathSize = strlen(ep->d_name);
        if (entryPathSize + baseDirLen + 1 > relPathBufferSize) {
            relPath = realloc(relPath, entryPathSize + baseDirLen + 1);
            if (relPath == NULL) {
                free(nearestFileName);
                goto cleanup_dp;
            }
        }
        /* strcat doesn't work because there's nothing saying there's a null at
         * relPath[baseDirLen + 1]. We're creating the file names by hand! */
        strcpy(&relPath[baseDirLen], ep->d_name);

        if (getDistance(relPath, inputImageVector, &newDistance) > 0)
            goto cleanup_all;
        if (newDistance < nearestDistance) {
            if (entryPathSize >= nearestFileNameSize) {
                nearestFileName = realloc(nearestFileName, entryPathSize + 1);
                if (nearestFileName == NULL) goto cleanup_rel_path;
                nearestFileNameSize = entryPathSize + 1;
            }
            strcpy(nearestFileName, ep->d_name);
            nearestDistance = newDistance;
        }
        errno = 0;
    }

    if (nearestFileName[0] != '\0') {
        *fileName = nearestFileName;
        *distance = nearestDistance;
        goto cleanup_rel_path;
    }

cleanup_all:
    free(nearestFileName);
cleanup_rel_path:
    free(relPath);
cleanup_dp:
    closedir(dp);

    return errno;
}
