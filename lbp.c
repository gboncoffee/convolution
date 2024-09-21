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
                GetPGMHeight(&inputImage), 255))
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
            vector[pixel]++;
        }
    }

    FreePGM(&image);

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
    int ret;
    if (makeSamePathWithLBPExtension(path, &lbpPath) > 0) return errno;
    if (!strcmp(path, lbpPath)) {
        free(lbpPath);
        return -1;
    }

    fp = fopen(lbpPath, "r");
    if (fp == NULL) {
        /* This error should simply be ignored because we may be trying to
         * search a file or something. */
        if (computeImageVector(path, vector)) {
            errno = 0;
            free(lbpPath);
            return -1;
        }
        ret = saveVectorFile(lbpPath, vector);
        free(lbpPath);
        return ret;
    }
    if (fread(vector, 1, 256, fp) != 256) {
        fclose(fp);
        /* Same. */
        if (computeImageVector(path, vector)) {
            errno = 0;
            free(lbpPath);
            return -1;
        }
        ret = saveVectorFile(lbpPath, vector);
        free(lbpPath);
        return ret;
    }

    fclose(fp);
    free(lbpPath);
    return 0;
}

int getDistance(char *path, uint8_t *baseVector, double *newDistance) {
    size_t i;
    double distance;
    int64_t cur;
    int ret;
    uint8_t *newVector = malloc(256);
    if (newVector == NULL) return errno;

    ret = GetImageVector(path, newVector);
    if (ret < 0) {
        /* Path is not a PGM.*/
        errno = 0;
        free(newVector);
        return -1;
    } else if (ret) {
        free(newVector);
        return errno;
    }

    /* Computes the distance. */
    distance = 0;
    for (i = 0; i < 256; i++) {
        cur = baseVector[i] - newVector[i];
        cur *= cur;
        distance += cur;
    }

    *newDistance = sqrt(distance);

    free(newVector);
    return 0;
}

int SearchBaseDirectory(char *baseDirectory, uint8_t *inputImageVector,
                        char *nearestFileName, double *distance) {
    struct dirent *ep;
    char *relPath;
    double nearestDistance;
    double newDistance;
    int ret;
    size_t baseDirLen = strlen(baseDirectory);

    DIR *dp = opendir(baseDirectory);
    if (dp == NULL) return errno;

    relPath = calloc(baseDirLen + 258, 1);
    if (relPath == NULL) {
        ret = errno;
        goto cleanup_dp;
    }
    strcpy(relPath, baseDirectory);
    if (relPath[baseDirLen - 1] != '/') {
        relPath[baseDirLen] = '/';
        relPath[baseDirLen + 1] = '\0';
        baseDirLen++;
    }

    /* Init the distance with HUGE_VAL for obvious reasons. */
    nearestDistance = HUGE_VAL;

    /* Iterate checking the distance. */
    while ((ep = readdir(dp))) {
        /* strcat doesn't work because there's nothing saying there's a null at
         * relPath[baseDirLen]. We're creating the file names by hand! */
        strcpy(&relPath[baseDirLen], ep->d_name);

        ret = getDistance(relPath, inputImageVector, &newDistance);
        if (ret > 0) {
            goto cleanup_rel_path;
        } else if (ret == 0 && newDistance < nearestDistance) {
            strcpy(nearestFileName, ep->d_name);
            nearestDistance = newDistance;
        }
    }

    if (nearestFileName[0] != '\0') {
        *distance = nearestDistance;
        ret = 0;
    } else {
        ret = -1;
    }

cleanup_rel_path:
    free(relPath);
cleanup_dp:
    closedir(dp);

    return ret;
}
