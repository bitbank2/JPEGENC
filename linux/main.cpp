//
//  JPEGENC test app
//
//  written by Larry Bank (bitbank@pobox.com)
//  Project started July 2021
//  Copyright (c) 2021 BitBank Software, Inc.
//  

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "../src/JPEGENC.h"
JPEGENC jpg; // static copy of JPEG encoder class

// If MEM_TO_MEM is defined, the encoder will output directly to a single buffer that you
// supply. If the buffer isn't large enough, it will exit before completing the image.
// If MEM_TO_MEM is not defined, it will write the output incrementally to file I/O callback functions
// that you provide.
//#define MEM_TO_MEM

//
// File I/O callback functions
// that you provide to work on your particular setup
// These are written for a Linux/POSIX file system
//
int32_t myWrite(JPEGE_FILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    FILE *ohandle = (FILE *)pFile->fHandle;
    return (int32_t)fwrite(pBuf, 1, iLen, ohandle);
} /* myWrite() */

int32_t myRead(JPEGE_FILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    FILE *ohandle = (FILE *)pFile->fHandle;
    return (int32_t)fread(pBuf, 1, iLen, ohandle);
} /* myRead() */

int32_t mySeek(JPEGE_FILE *pFile, int32_t iPosition)
{
    FILE *f = (FILE *)pFile->fHandle;
    fseek(f, iPosition, SEEK_SET);
    return(int32_t)ftell(f);

} /* mySeek() */

void * myOpen(const char *szFilename)
{
    return fopen(szFilename, "w+b");
} /* myOpen() */

void myClose(JPEGE_FILE *pHandle)
{
    FILE *f = (FILE *)pHandle->fHandle;
    fclose(f);
} /* myClose() */

//
// Read a Windows BMP file into memory
// For this demo, the only supported files are 24 or 32-bits per pixel
//
uint8_t * ReadBMP(const char *fname, int *width, int *height, int *bpp, unsigned char *pPal)
{
    int y, w, h, bits, offset;
    uint8_t *s, *d, *pTemp, *pBitmap;
    int pitch, bytewidth;
    int iSize, iDelta;
    FILE *infile;
    
    infile = fopen(fname, "r+b");
    if (infile == NULL) {
        printf("Error opening input file %s\n", fname);
        return NULL;
    }
    // Read the bitmap into RAM
    fseek(infile, 0, SEEK_END);
    iSize = (int)ftell(infile);
    fseek(infile, 0, SEEK_SET);
    pBitmap = (uint8_t *)malloc(iSize);
    pTemp = (uint8_t *)malloc(iSize);
    fread(pTemp, 1, iSize, infile);
    fclose(infile);
    
    if (pTemp[0] != 'B' || pTemp[1] != 'M' || pTemp[14] < 0x28) {
        free(pBitmap);
        free(pTemp);
        printf("Not a Windows BMP file!\n");
        return NULL;
    }
    w = *(int32_t *)&pTemp[18];
    h = *(int32_t *)&pTemp[22];
    bits = *(int16_t *)&pTemp[26] * *(int16_t *)&pTemp[28];
    if (bits <= 8) { // it has a palette, copy it
        free(pBitmap);
        free(pTemp);
        return NULL; // only support 24/32-bpp for now
//        uint8_t *p = pPal;
//        for (int i=0; i<(1<<bits); i++)
//        {
//           *p++ = pTemp[54+i*4];
//           *p++ = pTemp[55+i*4];
//           *p++ = pTemp[56+i*4];
//        }
    }
    offset = *(int32_t *)&pTemp[10]; // offset to bits
    bytewidth = (w * bits) >> 3;
    pitch = (bytewidth + 3) & 0xfffc; // DWORD aligned
// move up the pixels
    d = pBitmap;
    s = &pTemp[offset];
    iDelta = pitch;
    if (h > 0) {
        iDelta = -pitch;
        s = &pTemp[offset + (h-1) * pitch];
    } else {
        h = -h;
    }
    for (y=0; y<h; y++) {
        if (bits == 32) {// need to swap red and blue
            for (int i=0; i<bytewidth; i+=4) {
                d[i] = s[i+2];
                d[i+1] = s[i+1];
                d[i+2] = s[i];
                d[i+3] = s[i+3];
            }
        } else {
            memcpy(d, s, bytewidth);
        }
        d += bytewidth;
        s += iDelta;
    }
    *width = w;
    *height = h;
    *bpp = bits;
    free(pTemp);
    return pBitmap;
    
} /* ReadBMP() */

int main(int argc, const char * argv[]) {
#ifdef MEM_TO_MEM
    uint8_t *pBuffer;
    int iSize;
    FILE *oHandle;
#endif
    int i, rc, iWidth, iHeight;
    int iDataSize = 0, iMCUCount;
    JPEGENCODE jpe;
    uint8_t ucMCU[64]; // fake image data
    
    if (argc < 2 || argc > 3) {
        printf("Usage: jpegenc <infile.bmp> <outfile.jpg>\nor\n       jpegenc <outfile.jpg>\n");
        return -1;
    }
    if (argc == 2) { // dynamically generate an image
        iWidth = iHeight = 1024;
#ifdef MEM_TO_MEM
        iSize = 65536; // test with an output buffer
        pBuffer = (uint8_t *)malloc(iSize);
        rc = jpg.open(pBuffer, iSize);
#else // use incremental File I/O
        rc = jpg.open(argv[1], myOpen, myClose, myRead, myWrite, mySeek);
#endif
        if (rc == JPEGE_SUCCESS) {
            rc = jpg.encodeBegin(&jpe, iWidth, iHeight, JPEGE_PIXEL_GRAYSCALE, JPEGE_SUBSAMPLE_444, JPEGE_Q_BEST);
            if (rc == JPEGE_SUCCESS) {
                memset(ucMCU, 0, sizeof(ucMCU));
                for (i=0; i<8; i++)
                    ucMCU[i*8+i] = 192; // diagonal white line
                
                iMCUCount = ((iWidth + jpe.cx-1)/ jpe.cx) * ((iHeight + jpe.cy-1) / jpe.cy);
                for (i=0; i<iMCUCount && rc == JPEGE_SUCCESS; i++) {
                    // Send the same data for all MCUs (a simple diagonal line)
                    rc = jpg.addMCU(&jpe, ucMCU, 8);
                }
                iDataSize = jpg.close();
                printf("Output JPEG file size = %d bytes\n", iDataSize);
            }
#ifdef MEM_TO_MEM
            oHandle = fopen(argv[1], "w+b");
            if (oHandle) {
                fwrite(pBuffer, 1, iDataSize, oHandle);
                fclose(oHandle);
            }
            free(pBuffer);
#endif
        }
    } else { // convert BMP file into JPEG
        uint8_t *pBitmap, ucPixelType;
        int iBpp, iBytePP, iPitch;
        pBitmap = ReadBMP(argv[1], &iWidth, &iHeight, &iBpp, NULL);
        if (pBitmap == NULL)
        {
            fprintf(stderr, "Unable to open file: %s\n", argv[1]);
            return -1; // bad filename passed?
        }
        if (iBpp == 24) {
            iBytePP = 3;
            ucPixelType = JPEGE_PIXEL_RGB888;
        } else { // must be 32-bpp
            iBytePP = 4;
            ucPixelType = JPEGE_PIXEL_ARGB8888;
        }
        iPitch = iBytePP * iWidth;
#ifdef MEM_TO_MEM
        iSize = (iWidth * iHeight * 3)/4; // guesstimate of the output size
        pBuffer = (uint8_t *)malloc(iSize);
        rc = jpg.open(pBuffer, iSize);
#else // use incremental File I/O
        rc = jpg.open(argv[2], myOpen, myClose, myRead, myWrite, mySeek);
#endif
        if (rc == JPEGE_SUCCESS) {
            rc = jpg.encodeBegin(&jpe, iWidth, iHeight, ucPixelType, JPEGE_SUBSAMPLE_420, JPEGE_Q_BEST);
            if (rc == JPEGE_SUCCESS) {
                rc = jpg.addFrame(&jpe, pBitmap, iPitch);
                iDataSize = jpg.close();
            }
#ifdef MEM_TO_MEM
            // We captured the compressed data in a buffer, so we need to
            // explicitly write it into a file
            oHandle = fopen(argv[2], "w+b");
            if (oHandle) {
                fwrite(pBuffer, 1, iDataSize, oHandle);
                fclose(oHandle);
            }
            free(pBuffer);
#endif
            free(pBitmap);
        }
    }
    return 0;
} /* main() */
