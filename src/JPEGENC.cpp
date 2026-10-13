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

// The portable C code is compiled separately from jpegenc.c

//
// File (SD/MMC) based initialization
//
int JPEGENC::open(const char *szFilename, JPEGE_OPEN_CALLBACK *pfnOpen, JPEGE_CLOSE_CALLBACK *pfnClose, JPEGE_READ_CALLBACK *pfnRead, JPEGE_WRITE_CALLBACK *pfnWrite, JPEGE_SEEK_CALLBACK *pfnSeek)
{
    return JPEGENC_Open(&_jpeg, szFilename, pfnOpen, pfnClose, pfnRead, pfnWrite, pfnSeek);
} /* open() */

int JPEGENC::open(uint8_t *pOutput, int iBufferSize)
{
    return JPEGENC_OpenRAM(&_jpeg, pOutput, iBufferSize);
} /* open() */

//
// return the last error (if any)
//
int JPEGENC::getLastError()
{
    return JPEGENC_GetLastError(&_jpeg);
} /* getLastError() */
//
// Close the file - not needed when decoding from memory
//
int JPEGENC::close()
{
    return JPEGENC_Close(&_jpeg);
} /* close() */

int JPEGENC::encodeBegin(JPEGENCODE *pEncode, int iWidth, int iHeight, uint8_t ucPixelType, uint8_t ucSubSample, uint8_t ucQFactor)
{
    return JPEGENC_EncodeBegin(&_jpeg, pEncode, iWidth, iHeight, ucPixelType, ucSubSample, ucQFactor);
} /* encodeBegin() */

int JPEGENC::addMCU(JPEGENCODE *pEncode, uint8_t *pPixels, int iPitch)
{
    return JPEGENC_AddMCU(&_jpeg, pEncode, pPixels, iPitch);
} /* addMCU() */

int JPEGENC::addFrame(JPEGENCODE *pEncode, uint8_t *pPixels, int iPitch)
{
    return JPEGENC_AddFrame(&_jpeg, pEncode, pPixels, iPitch);
} /* addFrame() */

int JPEGENC::addFrameYUV420P(JPEGENCODE *pEncode, const uint8_t *pY, int iYPitch, const uint8_t *pCb, int iCbPitch, const uint8_t *pCr, int iCrPitch)
{
    return JPEGENC_AddFrameYUV420P(&_jpeg, pEncode, pY, iYPitch, pCb, iCbPitch, pCr, iCrPitch);
} /* addFrameYUV420P() */

int JPEGENC::addFrameNV12(JPEGENCODE *pEncode, const uint8_t *pY, int iYPitch, const uint8_t *pUV, int iUVPitch)
{
    return JPEGENC_AddFrameNV12(&_jpeg, pEncode, pY, iYPitch, pUV, iUVPitch);
} /* addFrameNV12() */

