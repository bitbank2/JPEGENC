// SPDX-FileCopyrightText: 2021 Larry Bank <bitbank@pobox.com>
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/JPEGENC.h"

#define OUTPUT_SIZE 65536

typedef struct edge_test_tag
{
    const char *name;
    int width;
    int height;
    int bytes_per_pixel;
    uint8_t pixel_type;
    uint8_t subsample;
} EDGE_TEST;

static int get_dimensions(const uint8_t *jpeg, int size, int *width, int *height)
{
    int i;

    for (i = 0; i + 8 < size; i++) {
        if (jpeg[i] == 0xff && jpeg[i + 1] == 0xc0) {
            *height = (jpeg[i + 5] << 8) | jpeg[i + 6];
            *width = (jpeg[i + 7] << 8) | jpeg[i + 8];
            return 0;
        }
    }

    return -1;
}

static int run_edge_test(const EDGE_TEST *test)
{
    JPEGE_IMAGE jpeg;
    JPEGENCODE encode;
    uint8_t *pixels;
    uint8_t *output;
    int width;
    int height;
    int input_size;
    int rc;
    int size;
    int i;
    int result = 1;

    input_size = test->width * test->height * test->bytes_per_pixel;
    pixels = malloc(input_size);
    output = malloc(OUTPUT_SIZE);
    if (pixels == NULL || output == NULL) {
        fprintf(stderr, "%s: allocation failed\n", test->name);
        goto out;
    }

    for (i = 0; i < input_size; i++)
        pixels[i] = (uint8_t)((i * 37) ^ (i << 5));

    rc = JPEGENC_OpenRAM(&jpeg, output, OUTPUT_SIZE);
    if (rc != JPEGE_SUCCESS) {
        fprintf(stderr, "%s: JPEGENC_OpenRAM failed: %d\n", test->name, rc);
        goto out;
    }

    rc = JPEGENC_EncodeBegin(&jpeg, &encode, test->width, test->height,
                         test->pixel_type, test->subsample, JPEGE_Q_BEST);
    if (rc != JPEGE_SUCCESS) {
        fprintf(stderr, "%s: JPEGENC_EncodeBegin failed: %d\n", test->name, rc);
        goto out;
    }

    rc = JPEGENC_AddFrame(&jpeg, &encode, pixels,
                      test->width * test->bytes_per_pixel);
    if (rc != JPEGE_SUCCESS) {
        fprintf(stderr, "%s: JPEGENC_AddFrame failed: %d\n", test->name, rc);
        goto out;
    }

    size = JPEGENC_Close(&jpeg);
    if (size < 4) {
        fprintf(stderr, "%s: JPEGENC_Close failed: %d\n", test->name,
                JPEGENC_GetLastError(&jpeg));
        goto out;
    }

    if (get_dimensions(output, size, &width, &height) != 0) {
        fprintf(stderr, "%s: JPEG dimensions not found\n", test->name);
        goto out;
    }
    if (width != test->width || height != test->height) {
        fprintf(stderr, "%s: wrong JPEG dimensions: %dx%d\n",
                test->name, width, height);
        goto out;
    }

    printf("edge MCU encoded %s %dx%d image: %d bytes\n",
           test->name, width, height, size);
    result = 0;

out:
    free(output);
    free(pixels);
    return result;
}

int main(void)
{
    static const EDGE_TEST tests[] = {
        { "grayscale-444", 9, 9, 1, JPEGE_PIXEL_GRAYSCALE, JPEGE_SUBSAMPLE_444 },
        { "rgb565-444", 9, 9, 2, JPEGE_PIXEL_RGB565_LE, JPEGE_SUBSAMPLE_444 },
        { "rgb888-444", 9, 9, 3, JPEGE_PIXEL_RGB888, JPEGE_SUBSAMPLE_444 },
        { "argb8888-444", 9, 9, 4, JPEGE_PIXEL_ARGB8888, JPEGE_SUBSAMPLE_444 },
        { "rgb565-420", 17, 17, 2, JPEGE_PIXEL_RGB565_LE, JPEGE_SUBSAMPLE_420 },
        { "rgb888-420", 17, 17, 3, JPEGE_PIXEL_RGB888, JPEGE_SUBSAMPLE_420 },
        { "argb8888-420", 17, 17, 4, JPEGE_PIXEL_ARGB8888, JPEGE_SUBSAMPLE_420 },
        { "yuv422-420", 17, 17, 2, JPEGE_PIXEL_YUV422, JPEGE_SUBSAMPLE_420 },
    };
    int i;

    for (i = 0; i < (int)(sizeof(tests) / sizeof(tests[0])); i++) {
        if (run_edge_test(&tests[i]) != 0)
            return 1;
    }

    return 0;
}
