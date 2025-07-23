//
//  main.cpp
//  JPEGENC_Test
//
//  Created by Laurence Bank on 3/10/25.
//

#include "../../../src/JPEGENC.cpp"
#include "rgb565.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

JPEGENC jpg;
JPEGENCODE jpe;
const char *pRootName = NULL;
uint8_t *pOut;
uint16_t u16Temp[320 * 24]; // hold 16 lines for capturing the MCUs (16x16)
FILE *f;
//
// Simple logging print
//
void JPEGLOG(int line, char *string, const char *result)
{
    printf("Line: %d: msg: %s%s\n", line, string, result);
} /* JPEGGLOG() */

void * myOpen(const char *filename) {
    f = fopen(filename, "w+b");
    return (void *)f;
}
void myClose(JPEGE_FILE *handle) {
    FILE *fh = (FILE *)handle->fHandle;
    if (fh != NULL) {
        fflush(fh);
        fclose(fh);
    }
}
int32_t myRead(JPEGE_FILE *handle, uint8_t *buffer, int32_t length) {
    FILE *fh = (FILE *)handle->fHandle;
    if (fh != NULL) {
        return (int32_t)fread(buffer, length, 1, fh);
    }
    return 0;
}
int32_t myWrite(JPEGE_FILE *handle, uint8_t *buffer, int32_t length) {
    FILE *fh = (FILE *)handle->fHandle;
    if (fh != NULL) {
        return (int32_t)fwrite(buffer, length, 1, fh);
    }
    return 0;
}
int32_t mySeek(JPEGE_FILE *handle, int32_t position) {
    FILE *fh = (FILE *)handle->fHandle;
    if (fh != NULL) {
        fseek(fh, position, SEEK_SET);
        return position;
    }
    return 0;
}

void SaveFile(const char *fname, uint8_t *pData, int iSize, int i)
{
    char szFilename[256];
    
    snprintf(szFilename, sizeof(szFilename), "%s%d.jpg", pRootName, i);
    f = fopen(szFilename, "w+b");
    if (f != NULL) {
        fwrite(pData, iSize, 1, f);
        fflush(f);
        fclose(f);
    }
} /* SaveFile() */

int main(int argc, const char * argv[]) {
    int x, y, k, rc, iTotal;
    char *szTestName;
    uint32_t u32;
    int iOutputSize;
    int iTotalPass, iTotalFail;
    const char *szStart = " - START";
    int iDataSize = 0;
    iTotalPass = iTotalFail = iTotal = 0;
    int w, h, offset, bytewidth, pitch;
    uint8_t r, g, b, *d;
    uint16_t *s;
    
    if (argc == 1) { // no parameters, show help
        printf("JPEGENC Test Suite\n");
        printf("Usage: jpegenc_test <optional root filename>\n");
        printf("If given a root filename (e.g. testoutput), it will save the\n");
        printf("output of tests to files named testoutput1.jpg, testoutput2.jpg, etc\n");
        printf("Where the number appended to the end is the test number\n");
    } else {
        pRootName = argv[1];
    }
    // Test 1
    iTotal++;
    szTestName = (char *)"Test encoding a RGB565 images as JPEG in RAM";
    JPEGLOG(__LINE__, szTestName, szStart);
    w = *(int32_t *)&rgb565[18];
    h = *(int32_t *)&rgb565[22];
//    bits = *(int16_t *)&rgb565[26] * *(int16_t *)&rgb565[28];
    iOutputSize = 65536;
    pOut = (uint8_t *)malloc(iOutputSize); // 64K should be plenty for this image
    offset = *(int32_t *)&rgb565[10]; // offset to bits
    bytewidth = (w * 2);
    pitch = (bytewidth + 3) & 0xfffc; // DWORD aligned
    s = (uint16_t *)&rgb565[offset];
    if (h > 0) { // postive = bottom-up bitmap
        s += (h-1)*(pitch/2);
        pitch = -pitch;
    } else {
        h = -h;
    }
    rc = jpg.open(pOut, iOutputSize);
    if (rc == JPEGE_SUCCESS) {
        rc = jpg.encodeBegin(&jpe, w, h, JPEGE_PIXEL_RGB565, JPEGE_SUBSAMPLE_420, JPEGE_Q_HIGH);
        if (rc == JPEGE_SUCCESS) {
            for (y=0; y<h; y+=16) { // MCUs are 16 lines tall for 4:2:0 subsampling
                for (k=0; k<16; k++) {
                    memcpy(&u16Temp[k*w], s, w*sizeof(uint16_t)); // JPEGENC expects little endian data
                    s += pitch/sizeof(uint16_t);
                } // for k
               for (x=0; x < w; x+= 16) {
                  rc = jpg.addMCU(&jpe, (uint8_t *)&u16Temp[x], w * sizeof(uint16_t)); // encode this row of MCUs
               } // for each MCU
            } // for y
            iDataSize = jpg.close();
            u32 = *(uint32_t *)pOut;
            if (iDataSize == 11076 && u32 == 0xe0ffd8ff) { // correct length the start of the JPEG header
                iTotalPass++;
                JPEGLOG(__LINE__, szTestName, " - PASSED");
            } else {
                iTotalFail++;
                JPEGLOG(__LINE__, szTestName, " - FAILED");
            }
        } else {
            iTotalFail++;
            JPEGLOG(__LINE__, szTestName, " - FAILED");
        }
    } else {
        JPEGLOG(__LINE__, szTestName, "Error creating JPEG file.");
    }
    if (pRootName) {
        SaveFile(pRootName, pOut, iDataSize, iTotal);
    }
    free(pOut);
    // Test 2
    iTotal++;
    szTestName = (char *)"Test encoding the same image as RGB888";
    JPEGLOG(__LINE__, szTestName, szStart);
    w = *(int32_t *)&rgb565[18];
    h = *(int32_t *)&rgb565[22];
//    bits = *(int16_t *)&rgb565[26] * *(int16_t *)&rgb565[28];
    iOutputSize = 65536;
    pOut = (uint8_t *)malloc(iOutputSize); // 64K should be plenty for this image
    offset = *(int32_t *)&rgb565[10]; // offset to bits
    bytewidth = (w * 2);
    pitch = (bytewidth + 3) & 0xfffc; // DWORD aligned
    s = (uint16_t *)&rgb565[offset];
    if (h > 0) {
        s += (h-1) * (pitch/2);
        pitch = -pitch;
    } else {
        h = -h;
    }
    rc = jpg.open(pOut, iOutputSize);
    if (rc == JPEGE_SUCCESS) {
        rc = jpg.encodeBegin(&jpe, w, h, JPEGE_PIXEL_BGR888, JPEGE_SUBSAMPLE_420, JPEGE_Q_HIGH);
        if (rc == JPEGE_SUCCESS) {
            for (y=0; y<h; y+=16) { // MCUs are 16 lines tall for 4:2:0 subsampling
                for (k=0; k<16; k++) {
                    d = (uint8_t *)u16Temp;
                    d += k * w * 3;
                    for (x=0; x<w; x++) { // convert each RGB565 pixel to RGB888
                        b = (unsigned char)(((s[x] & 0x1f)<<3) | (s[x] & 7));
                        g = (unsigned char)(((s[x] & 0x7e0)>>3) | ((s[x] & 0x60)>>5));
                        r = (unsigned char)(((s[x] & 0xf800)>>8) | ((s[x] & 0x3800)>>11));
                        d[0] = b; d[1] = g; d[2] = r;
                        d += 3;
                    }
                    s += pitch/sizeof(uint16_t);
                } // for k
                d = (uint8_t *)u16Temp;
               for (x=0; x < w; x+= 16) {
                  rc = jpg.addMCU(&jpe, (uint8_t *)&d[x*3], w * 3); // encode this row of MCUs
               } // for each MCU
            } // for y
            iDataSize = jpg.close();
            u32 = *(uint32_t *)pOut;
            if (iDataSize == 11076 && u32 == 0xe0ffd8ff) { // correct length the start of the JPEG header
                iTotalPass++;
                JPEGLOG(__LINE__, szTestName, " - PASSED");
            } else {
                iTotalFail++;
                JPEGLOG(__LINE__, szTestName, " - FAILED");
            }
        } else {
            iTotalFail++;
            JPEGLOG(__LINE__, szTestName, " - FAILED");
        }
    } else {
        JPEGLOG(__LINE__, szTestName, "Error creating JPEG file.");
    }
    if (pRootName) {
        SaveFile(pRootName, pOut, iDataSize, iTotal);
    }
    free(pOut);
    // Test 3
    iTotal++;
    szTestName = (char *)"Test encoding the same image as 8-bit grayscale";
    JPEGLOG(__LINE__, szTestName, szStart);
    w = *(int32_t *)&rgb565[18];
    h = *(int32_t *)&rgb565[22];
//    bits = *(int16_t *)&rgb565[26] * *(int16_t *)&rgb565[28];
    iOutputSize = 65536;
    pOut = (uint8_t *)malloc(iOutputSize); // 64K should be plenty for this image
    offset = *(int32_t *)&rgb565[10]; // offset to bits
    bytewidth = (w * 2);
    pitch = (bytewidth + 3) & 0xfffc; // DWORD aligned
    s = (uint16_t *)&rgb565[offset];
    if (h > 0) {
        s += (h-1) * (pitch/2);
        pitch = -pitch;
    } else {
        h = -h;
    }
    rc = jpg.open(pOut, iOutputSize);
    if (rc == JPEGE_SUCCESS) {
        rc = jpg.encodeBegin(&jpe, w, h, JPEGE_PIXEL_GRAYSCALE, JPEGE_SUBSAMPLE_444, JPEGE_Q_HIGH);
        if (rc == JPEGE_SUCCESS) {
            for (y=0; y<h; y+=8) { // MCUs are 8 lines tall for grayscale
                for (k=0; k<8; k++) {
                    d = (uint8_t *)u16Temp;
                    d += k * w;
                    for (x=0; x<w; x++) { // convert each RGB565 pixel to RGB888
                        b = (unsigned char)(((s[x] & 0x1f)<<3) | (s[x] & 7));
                        g = (unsigned char)(((s[x] & 0x7e0)>>3) | ((s[x] & 0x60)>>5));
                        r = (unsigned char)(((s[x] & 0xf800)>>8) | ((s[x] & 0x3800)>>11));
                        *d++ = (b+(g*2)+r)/4;
                    }
                    s += pitch/sizeof(uint16_t);
                } // for k
                d = (uint8_t *)u16Temp;
                for (x=0; x < w; x+= 8) {
                    rc = jpg.addMCU(&jpe, (uint8_t *)&d[x], w); // encode this row of MCUs
               } // for each MCU
            } // for y
            iDataSize = jpg.close();
            u32 = *(uint32_t *)pOut;
            if (iDataSize == 9561 && u32 == 0xe0ffd8ff) { // correct length the start of the JPEG header
                iTotalPass++;
                JPEGLOG(__LINE__, szTestName, " - PASSED");
            } else {
                iTotalFail++;
                JPEGLOG(__LINE__, szTestName, " - FAILED");
            }
        } else {
            iTotalFail++;
            JPEGLOG(__LINE__, szTestName, " - FAILED");
        }
    } else {
        JPEGLOG(__LINE__, szTestName, "Error creating JPEG file.");
    }
    if (pRootName) {
        SaveFile(pRootName, pOut, iDataSize, iTotal);
    }
    free(pOut);
    // Test 4
    iTotal++;
    szTestName = (char *)"Test invalid parameters 1";
    JPEGLOG(__LINE__, szTestName, szStart);
    rc = jpg.open(NULL, 0);
    if (rc == JPEGE_INVALID_PARAMETER) {
        iTotalPass++;
        JPEGLOG(__LINE__, szTestName, " - PASSED");
    } else {
        iTotalFail++;
        JPEGLOG(__LINE__, szTestName, " - FAILED");
    }
    // Test 5
    iTotal++;
    szTestName = (char *)"Test invalid parameters 2";
    JPEGLOG(__LINE__, szTestName, szStart);
    iOutputSize = 65536;
    pOut = (uint8_t *)malloc(iOutputSize); // 64K should be plenty for this image
    rc = jpg.open(pOut, iOutputSize);
    if (rc == JPEGE_SUCCESS) { // check for invalid size
        rc = jpg.encodeBegin(&jpe, 0, 0, JPEGE_PIXEL_GRAYSCALE, JPEGE_SUBSAMPLE_444, JPEGE_Q_HIGH);
        if (rc == JPEGE_INVALID_PARAMETER) {
            iTotalPass++;
            JPEGLOG(__LINE__, szTestName, " - PASSED");
        } else {
            iTotalFail++;
            JPEGLOG(__LINE__, szTestName, " - FAILED");
        }
    }
    // Test 6
    iTotal++;
    szTestName = (char *)"Test invalid filename pointers";
    JPEGLOG(__LINE__, szTestName, szStart);
    rc = jpg.open(NULL, myOpen, myClose, myRead, myWrite, mySeek);
    if (rc == JPEGE_INVALID_PARAMETER) {
        iTotalPass++;
        JPEGLOG(__LINE__, szTestName, " - PASSED");
    } else {
        iTotalFail++;
        JPEGLOG(__LINE__, szTestName, " - FAILED");
    }
    // Test 7
    iTotal++;
    szTestName = (char *)"Test invalid file callback pointers";
    JPEGLOG(__LINE__, szTestName, szStart);
    rc = jpg.open(pRootName, NULL, NULL, NULL, NULL, NULL);
    if (rc == JPEGE_INVALID_PARAMETER) {
        iTotalPass++;
        JPEGLOG(__LINE__, szTestName, " - PASSED");
    } else {
        iTotalFail++;
        JPEGLOG(__LINE__, szTestName, " - FAILED");
    }

    if (pRootName) { // Test writing to the file callbacks
        // Test 8
        char szFile[256];
        iTotal++;
        snprintf(szFile, sizeof(szFile), "%s%d.jpg", pRootName, iTotal);
        szTestName = (char *)"Test write a file with callback functions";
        JPEGLOG(__LINE__, szTestName, szStart);
        w = *(int32_t *)&rgb565[18];
        h = *(int32_t *)&rgb565[22];
        offset = *(int32_t *)&rgb565[10]; // offset to bits
        bytewidth = (w * 2);
        pitch = (bytewidth + 3) & 0xfffc; // DWORD aligned
        s = (uint16_t *)&rgb565[offset];
        if (h > 0) { // postive = bottom-up bitmap
            s += (h-1)*(pitch/2);
            pitch = -pitch;
        } else {
            h = -h;
        }
        rc = jpg.open(szFile, myOpen, myClose, myRead, myWrite, mySeek);
        if (rc == JPEGE_SUCCESS) {
            rc = jpg.encodeBegin(&jpe, w, h, JPEGE_PIXEL_RGB565, JPEGE_SUBSAMPLE_420, JPEGE_Q_HIGH);
            if (rc == JPEGE_SUCCESS) {
                for (y=0; y<h; y+=16) { // MCUs are 16 lines tall for 4:2:0 subsampling
                    for (k=0; k<16; k++) {
                        memcpy(&u16Temp[k*w], s, w*sizeof(uint16_t)); // JPEGENC expects little endian data
                        s += pitch/sizeof(uint16_t);
                    } // for k
                    for (x=0; x < w; x+= 16) {
                        rc = jpg.addMCU(&jpe, (uint8_t *)&u16Temp[x], w * sizeof(uint16_t)); // encode this row of MCUs
                    } // for each MCU
                } // for y
                iDataSize = jpg.close();
                u32 = *(uint32_t *)pOut;
                if (iDataSize == 11076 && u32 == 0xe0ffd8ff) { // correct length the start of the JPEG header
                    iTotalPass++;
                    JPEGLOG(__LINE__, szTestName, " - PASSED");
                } else {
                    iTotalFail++;
                    JPEGLOG(__LINE__, szTestName, " - FAILED");
                }
            } else {
                iTotalFail++;
                JPEGLOG(__LINE__, szTestName, " - FAILED");
            }
        } else {
            JPEGLOG(__LINE__, szTestName, "Error creating JPEG file.");
        }
    }

    printf("Total tests: %d, %d passed, %d failed\n", iTotal, iTotalPass, iTotalFail);
    return 0;
}
