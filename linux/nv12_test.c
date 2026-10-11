// SPDX-FileCopyrightText: 2021 Larry Bank <bitbank@pobox.com>
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/JPEGENC.h"

#define OUTPUT_SIZE 65536

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

static int encode_yuv420p(const uint8_t *y, int y_pitch,
                          const uint8_t *cb, int cb_pitch,
                          const uint8_t *cr, int cr_pitch,
                          int width, int height,
                          uint8_t *output, int *output_size)
{
    JPEGE_IMAGE jpeg;
    JPEGENCODE encode;
    int rc;

    rc = JPEGOpenRAM(&jpeg, output, OUTPUT_SIZE);
    if (rc == JPEGE_SUCCESS) {
        rc = JPEGEncodeBegin(&jpeg, &encode, width, height,
                             JPEGE_PIXEL_YUV420P, JPEGE_SUBSAMPLE_420,
                             JPEGE_Q_BEST);
    }
    if (rc == JPEGE_SUCCESS) {
        rc = JPEGAddFrameYUV420P(&jpeg, &encode, y, y_pitch, cb, cb_pitch,
                                 cr, cr_pitch);
    }
    if (rc != JPEGE_SUCCESS) {
        fprintf(stderr, "YUV420P encode failed: %d\n", rc);
        return -1;
    }

    *output_size = JPEGClose(&jpeg);
    return *output_size > 0 ? 0 : -1;
}

static int encode_nv12(const uint8_t *y, int y_pitch,
                       const uint8_t *uv, int uv_pitch,
                       int width, int height,
                       uint8_t *output, int *output_size)
{
    JPEGE_IMAGE jpeg;
    JPEGENCODE encode;
    int rc;

    rc = JPEGOpenRAM(&jpeg, output, OUTPUT_SIZE);
    if (rc == JPEGE_SUCCESS) {
        rc = JPEGEncodeBegin(&jpeg, &encode, width, height,
                             JPEGE_PIXEL_NV12, JPEGE_SUBSAMPLE_420,
                             JPEGE_Q_BEST);
    }
    if (rc == JPEGE_SUCCESS) {
        rc = JPEGAddFrameNV12(&jpeg, &encode, y, y_pitch, uv, uv_pitch);
    }
    if (rc != JPEGE_SUCCESS) {
        fprintf(stderr, "NV12 encode failed: %d\n", rc);
        return -1;
    }

    *output_size = JPEGClose(&jpeg);
    return *output_size > 0 ? 0 : -1;
}

static int test_yuv420p_equivalence(void)
{
    enum { WIDTH = 16, HEIGHT = 16, CHROMA_WIDTH = 8, CHROMA_HEIGHT = 8 };
    uint8_t y[WIDTH * HEIGHT];
    uint8_t cb[CHROMA_WIDTH * CHROMA_HEIGHT];
    uint8_t cr[CHROMA_WIDTH * CHROMA_HEIGHT];
    uint8_t uv[CHROMA_WIDTH * CHROMA_HEIGHT * 2];
    uint8_t yuv420p_jpeg[OUTPUT_SIZE];
    uint8_t nv12_jpeg[OUTPUT_SIZE];
    int yuv420p_size;
    int nv12_size;
    int i;

    for (i = 0; i < WIDTH * HEIGHT; i++)
        y[i] = (uint8_t)(16 + ((i * 13) % 220));
    for (i = 0; i < CHROMA_WIDTH * CHROMA_HEIGHT; i++) {
        cb[i] = (uint8_t)(64 + i);
        cr[i] = (uint8_t)(192 - i);
        uv[i * 2] = cb[i];
        uv[i * 2 + 1] = cr[i];
    }

    if (encode_yuv420p(y, WIDTH, cb, CHROMA_WIDTH, cr, CHROMA_WIDTH,
                       WIDTH, HEIGHT, yuv420p_jpeg, &yuv420p_size) != 0 ||
        encode_nv12(y, WIDTH, uv, CHROMA_WIDTH * 2, WIDTH, HEIGHT,
                    nv12_jpeg, &nv12_size) != 0) {
        return -1;
    }

    if (yuv420p_size != nv12_size ||
        memcmp(yuv420p_jpeg, nv12_jpeg, yuv420p_size) != 0) {
        fprintf(stderr, "NV12 output differs from equivalent YUV420P input\n");
        return -1;
    }

    printf("NV12 matches equivalent YUV420P input: %d bytes\n", nv12_size);
    return 0;
}

static int test_odd_dimensions(void)
{
    enum { WIDTH = 17, HEIGHT = 17, CHROMA_WIDTH = 9, CHROMA_HEIGHT = 9 };
    uint8_t *y = malloc(WIDTH * HEIGHT);
    uint8_t *uv = malloc(CHROMA_WIDTH * CHROMA_HEIGHT * 2);
    uint8_t *output = malloc(OUTPUT_SIZE);
    int output_size;
    int width;
    int height;
    int i;
    int result = -1;

    if (y == NULL || uv == NULL || output == NULL) {
        fprintf(stderr, "allocation failed\n");
        goto out;
    }

    for (i = 0; i < WIDTH * HEIGHT; i++)
        y[i] = (uint8_t)(i * 13);
    for (i = 0; i < CHROMA_WIDTH * CHROMA_HEIGHT; i++) {
        uv[i * 2] = (uint8_t)(96 + i);
        uv[i * 2 + 1] = (uint8_t)(160 - i);
    }

    if (encode_nv12(y, WIDTH, uv, CHROMA_WIDTH * 2, WIDTH, HEIGHT,
                    output, &output_size) != 0) {
        goto out;
    }
    if (get_dimensions(output, output_size, &width, &height) != 0 ||
        width != WIDTH || height != HEIGHT) {
        fprintf(stderr, "wrong NV12 JPEG dimensions\n");
        goto out;
    }

    printf("NV12 encoded odd-size %dx%d image: %d bytes\n",
           width, height, output_size);
    result = 0;

out:
    free(output);
    free(uv);
    free(y);
    return result;
}

int main(void)
{
    if (test_yuv420p_equivalence() != 0 || test_odd_dimensions() != 0)
        return 1;

    return 0;
}
