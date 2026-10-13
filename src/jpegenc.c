// SPDX-FileCopyrightText: 2021 Larry Bank <bitbank@pobox.com>
// SPDX-License-Identifier: Apache-2.0
//
// JPEG Encoder
//
// Arduino port started 7/22/2021
// Original JPEG code written 20+ years ago :)
// The goal of this code is to encode JPEG images on embedded systems
//
// Copyright 2021 BitBank Software, Inc. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//===========================================================================
//
#include "JPEGENC.h"

//
// File (SD/MMC) based initialization
//
int JPEGENC_Open(JPEGE_IMAGE *pJPEG, const char *szFilename, JPEGE_OPEN_CALLBACK *pfnOpen, JPEGE_CLOSE_CALLBACK *pfnClose, JPEGE_READ_CALLBACK *pfnRead, JPEGE_WRITE_CALLBACK *pfnWrite, JPEGE_SEEK_CALLBACK *pfnSeek)
{
    if (pJPEG == NULL || szFilename == NULL || pfnOpen == NULL || pfnClose == NULL || pfnRead == NULL || pfnWrite == NULL || pfnSeek == NULL) {
        return JPEGE_INVALID_PARAMETER;
    }
    memset(pJPEG, 0, sizeof(JPEGE_IMAGE));
    pJPEG->pfnRead = pfnRead;
    pJPEG->pfnWrite = pfnWrite;
    pJPEG->pfnSeek = pfnSeek;
    pJPEG->pfnOpen = pfnOpen;
    pJPEG->pfnClose = pfnClose;
    pJPEG->JPEGFile.fHandle = (*pfnOpen)(szFilename);
    pJPEG->pHighWater = &pJPEG->ucFileBuf[JPEGE_FILE_BUF_SIZE - 512];
    if (pJPEG->JPEGFile.fHandle == NULL) {
        pJPEG->iError = JPEGE_INVALID_FILE;
       return JPEGE_INVALID_FILE;
    }
    return JPEGE_SUCCESS;

} /* JPEGENC_Open() */

int JPEGENC_OpenRAM(JPEGE_IMAGE *pJPEG, uint8_t *pOutput, int iBufferSize)
{
    if (pJPEG == NULL || pOutput == NULL || iBufferSize < 1024) return JPEGE_INVALID_PARAMETER;
    memset(pJPEG, 0, sizeof(JPEGE_IMAGE));
    pJPEG->pOutput = pOutput;
    pJPEG->iBufferSize = iBufferSize;
    pJPEG->pHighWater = &pOutput[iBufferSize - 512];

    return JPEGE_SUCCESS;
} /* JPEGENC_OpenRAM() */

//
// return the last error (if any)
//
int JPEGENC_GetLastError(JPEGE_IMAGE *pJPEG)
{
    if (pJPEG == NULL) return JPEGE_INVALID_PARAMETER;
    return pJPEG->iError;
} /* JPEGENC_GetLastError() */

//
// Close the file - not needed when encoding to memory
//
int JPEGENC_Close(JPEGE_IMAGE *pJPEG)
{
    if (pJPEG == NULL) return JPEGE_INVALID_PARAMETER;
    JPEGENC_EncodeEnd(pJPEG);
    if (pJPEG->pfnClose)
        (*pJPEG->pfnClose)(&pJPEG->JPEGFile);
    return pJPEG->iDataSize;
} /* JPEGENC_Close() */

// Include the C code which does the actual work
#include "jpegenc.inl"
