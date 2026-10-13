// SPDX-FileCopyrightText: Copyright The JPEGEnc Contributors
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/JPEGENC.h"

#define OUTPUT_SIZE 65536

static int encode(const uint8_t *pixels, int width, int height, uint8_t pixel_type,
                  uint8_t subsample, uint8_t *output)
{
    JPEGE_IMAGE jpeg;
    JPEGENCODE encode_state;
    int rc;

    rc = JPEGENC_OpenRAM(&jpeg, output, OUTPUT_SIZE);
    if (rc != JPEGE_SUCCESS)
        return -1;

    rc = JPEGENC_EncodeBegin(&jpeg, &encode_state, width, height, pixel_type,
                             subsample, JPEGE_Q_BEST);
    if (rc != JPEGE_SUCCESS)
        return -1;

    rc = JPEGENC_AddFrame(&jpeg, &encode_state, (uint8_t *)pixels, width * 2);
    if (rc != JPEGE_SUCCESS)
        return -1;

    return JPEGENC_Close(&jpeg);
}

static int run_case(int width, int height, uint8_t subsample)
{
    size_t pixel_count = (size_t)width * (size_t)height;
    uint8_t *pixels_le = malloc(pixel_count * 2);
    uint8_t *pixels_be = malloc(pixel_count * 2);
    uint8_t *output_le = malloc(OUTPUT_SIZE);
    uint8_t *output_be = malloc(OUTPUT_SIZE);
    int size_le;
    int size_be;
    size_t i;
    int result = 1;

    if (pixels_le == NULL || pixels_be == NULL || output_le == NULL || output_be == NULL) {
        fprintf(stderr, "allocation failed\n");
        goto out;
    }

    for (i = 0; i < pixel_count; i++) {
        uint16_t pixel = (uint16_t)((((i * 3) & 31) << 11) |
                                    (((i * 5) & 63) << 5) |
                                    ((i * 7) & 31));

        pixels_le[i * 2] = (uint8_t)pixel;
        pixels_le[i * 2 + 1] = (uint8_t)(pixel >> 8);
        pixels_be[i * 2] = (uint8_t)(pixel >> 8);
        pixels_be[i * 2 + 1] = (uint8_t)pixel;
    }

    size_le = encode(pixels_le, width, height, JPEGE_PIXEL_RGB565_LE, subsample,
                     output_le);
    size_be = encode(pixels_be, width, height, JPEGE_PIXEL_RGB565_BE, subsample,
                     output_be);
    if (size_le <= 0 || size_be <= 0) {
        fprintf(stderr, "%dx%d encoding failed: LE=%d BE=%d\n",
                width, height, size_le, size_be);
        goto out;
    }
    if (size_le != size_be || memcmp(output_le, output_be, (size_t)size_le) != 0) {
        fprintf(stderr, "%dx%d LE/BE JPEG output differs: LE=%d BE=%d\n",
                width, height, size_le, size_be);
        goto out;
    }

    printf("RGB565 LE/BE %dx%d outputs match: %d bytes\n", width, height, size_le);
    result = 0;

out:
    free(output_be);
    free(output_le);
    free(pixels_be);
    free(pixels_le);
    return result;
}

int main(void)
{
    if (run_case(9, 9, JPEGE_SUBSAMPLE_444) != 0)
        return 1;
    if (run_case(17, 17, JPEGE_SUBSAMPLE_420) != 0)
        return 1;
    return 0;
}
