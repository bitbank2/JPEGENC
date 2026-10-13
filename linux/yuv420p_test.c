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

static int encode_yuv422(const uint8_t *pixels, int width, int height,
                         uint8_t *output, int *output_size)
{
    JPEGE_IMAGE jpeg;
    JPEGENCODE encode;
    int rc;

    rc = JPEGENC_OpenRAM(&jpeg, output, OUTPUT_SIZE);
    if (rc == JPEGE_SUCCESS) {
        rc = JPEGENC_EncodeBegin(&jpeg, &encode, width, height,
                             JPEGE_PIXEL_YUV422, JPEGE_SUBSAMPLE_420,
                             JPEGE_Q_BEST);
    }
    if (rc == JPEGE_SUCCESS) {
        rc = JPEGENC_AddFrame(&jpeg, &encode, (uint8_t *)pixels, width * 2);
    }
    if (rc != JPEGE_SUCCESS) {
        fprintf(stderr, "YUV422 encode failed: %d\n", rc);
        return -1;
    }

    *output_size = JPEGENC_Close(&jpeg);
    return *output_size > 0 ? 0 : -1;
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

    rc = JPEGENC_OpenRAM(&jpeg, output, OUTPUT_SIZE);
    if (rc == JPEGE_SUCCESS) {
        rc = JPEGENC_EncodeBegin(&jpeg, &encode, width, height,
                             JPEGE_PIXEL_YUV420P, JPEGE_SUBSAMPLE_420,
                             JPEGE_Q_BEST);
    }
    if (rc == JPEGE_SUCCESS) {
        rc = JPEGENC_AddFrameYUV420P(&jpeg, &encode, y, y_pitch, cb, cb_pitch,
                                cr, cr_pitch);
    }
    if (rc != JPEGE_SUCCESS) {
        fprintf(stderr, "YUV420P encode failed: %d\n", rc);
        return -1;
    }

    *output_size = JPEGENC_Close(&jpeg);
    return *output_size > 0 ? 0 : -1;
}

static int test_yuv422_equivalence(void)
{
    enum { WIDTH = 16, HEIGHT = 16, CHROMA_WIDTH = 8 };
    uint8_t y[WIDTH * HEIGHT];
    uint8_t cb[CHROMA_WIDTH * (HEIGHT / 2)];
    uint8_t cr[CHROMA_WIDTH * (HEIGHT / 2)];
    uint8_t packed[WIDTH * HEIGHT * 2];
    uint8_t yuv422_jpeg[OUTPUT_SIZE];
    uint8_t yuv420p_jpeg[OUTPUT_SIZE];
    int yuv422_size;
    int yuv420p_size;
    int row;
    int column;

    for (row = 0; row < HEIGHT; row++) {
        for (column = 0; column < WIDTH; column++) {
            y[row * WIDTH + column] =
                (uint8_t)(16 + ((row * 11 + column * 7) % 220));
        }
    }
    for (row = 0; row < HEIGHT / 2; row++) {
        for (column = 0; column < CHROMA_WIDTH; column++) {
            cb[row * CHROMA_WIDTH + column] =
                (uint8_t)(64 + row * 5 + column * 3);
            cr[row * CHROMA_WIDTH + column] =
                (uint8_t)(192 - row * 4 - column * 2);
        }
    }

    for (row = 0; row < HEIGHT; row++) {
        for (column = 0; column < WIDTH; column += 2) {
            int packed_offset = row * WIDTH * 2 + column * 2;
            int chroma_offset = (row / 2) * CHROMA_WIDTH + column / 2;

            packed[packed_offset] = y[row * WIDTH + column];
            packed[packed_offset + 1] = cb[chroma_offset];
            packed[packed_offset + 2] = y[row * WIDTH + column + 1];
            packed[packed_offset + 3] = cr[chroma_offset];
        }
    }

    if (encode_yuv422(packed, WIDTH, HEIGHT, yuv422_jpeg,
                      &yuv422_size) != 0 ||
        encode_yuv420p(y, WIDTH, cb, CHROMA_WIDTH, cr, CHROMA_WIDTH,
                      WIDTH, HEIGHT, yuv420p_jpeg, &yuv420p_size) != 0) {
        return -1;
    }

    if (yuv422_size != yuv420p_size ||
        memcmp(yuv422_jpeg, yuv420p_jpeg, yuv422_size) != 0) {
        fprintf(stderr, "YUV420P output differs from equivalent YUV422 input\n");
        return -1;
    }

    printf("YUV420P matches equivalent YUV422 input: %d bytes\n",
           yuv420p_size);
    return 0;
}

static int test_odd_dimensions(void)
{
    enum { WIDTH = 17, HEIGHT = 17, CHROMA_WIDTH = 9, CHROMA_HEIGHT = 9 };
    uint8_t *y = malloc(WIDTH * HEIGHT);
    uint8_t *cb = malloc(CHROMA_WIDTH * CHROMA_HEIGHT);
    uint8_t *cr = malloc(CHROMA_WIDTH * CHROMA_HEIGHT);
    uint8_t *output = malloc(OUTPUT_SIZE);
    int output_size;
    int width;
    int height;
    int i;
    int result = -1;

    if (y == NULL || cb == NULL || cr == NULL || output == NULL) {
        fprintf(stderr, "allocation failed\n");
        goto out;
    }

    for (i = 0; i < WIDTH * HEIGHT; i++)
        y[i] = (uint8_t)(i * 13);
    for (i = 0; i < CHROMA_WIDTH * CHROMA_HEIGHT; i++) {
        cb[i] = (uint8_t)(96 + i);
        cr[i] = (uint8_t)(160 - i);
    }

    if (encode_yuv420p(y, WIDTH, cb, CHROMA_WIDTH, cr, CHROMA_WIDTH,
                      WIDTH, HEIGHT, output, &output_size) != 0) {
        goto out;
    }
    if (get_dimensions(output, output_size, &width, &height) != 0 ||
        width != WIDTH || height != HEIGHT) {
        fprintf(stderr, "wrong YUV420P JPEG dimensions\n");
        goto out;
    }

    printf("YUV420P encoded odd-size %dx%d image: %d bytes\n",
           width, height, output_size);
    result = 0;

out:
    free(output);
    free(cr);
    free(cb);
    free(y);
    return result;
}

int main(void)
{
    if (test_yuv422_equivalence() != 0 || test_odd_dimensions() != 0)
        return 1;

    return 0;
}
