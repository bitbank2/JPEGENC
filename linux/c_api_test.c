// SPDX-FileCopyrightText: 2021 Larry Bank <bitbank@pobox.com>
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <stdio.h>

#include "../src/JPEGENC.h"

#define IMAGE_WIDTH 16
#define IMAGE_HEIGHT 16
#define OUTPUT_SIZE 4096

int main(void)
{
    JPEGE_IMAGE jpeg;
    JPEGENCODE encode;
    uint8_t pixels[IMAGE_WIDTH * IMAGE_HEIGHT];
    uint8_t output[OUTPUT_SIZE];
    int i;
    int rc;
    int size;

    for (i = 0; i < (int)sizeof(pixels); i++) {
        pixels[i] = (uint8_t)i;
    }

    rc = JPEGENC_OpenRAM(&jpeg, output, sizeof(output));
    if (rc != JPEGE_SUCCESS) {
        fprintf(stderr, "JPEGENC_OpenRAM failed: %d\n", rc);
        return 1;
    }

    rc = JPEGENC_EncodeBegin(&jpeg, &encode, IMAGE_WIDTH, IMAGE_HEIGHT,
                         JPEGE_PIXEL_GRAYSCALE, JPEGE_SUBSAMPLE_444,
                         JPEGE_Q_BEST);
    if (rc != JPEGE_SUCCESS) {
        fprintf(stderr, "JPEGENC_EncodeBegin failed: %d\n", rc);
        return 1;
    }

    rc = JPEGENC_AddFrame(&jpeg, &encode, pixels, IMAGE_WIDTH);
    if (rc != JPEGE_SUCCESS) {
        fprintf(stderr, "JPEGENC_AddFrame failed: %d\n", rc);
        return 1;
    }

    size = JPEGENC_Close(&jpeg);
    if (size < 4) {
        fprintf(stderr, "JPEGENC_Close failed: %d\n", JPEGENC_GetLastError(&jpeg));
        return 1;
    }

    if (output[0] != 0xff || output[1] != 0xd8 ||
        output[size - 2] != 0xff || output[size - 1] != 0xd9) {
        fprintf(stderr, "invalid JPEG markers\n");
        return 1;
    }

    printf("C API encoded %dx%d grayscale image: %d bytes\n",
           IMAGE_WIDTH, IMAGE_HEIGHT, size);
    return 0;
}
