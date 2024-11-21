//
// This example demonstrates how to create a 24-bit color JPEG file
// from RGB565 pixels (A Windows BMP file). The result is written
// to a file on a microSD card. This example uses the JC4827W543
// ESP32-S3 LCD board from Guition, but any target system will work.
//
#include "rgb565.h"
#include <bb_spi_lcd.h>
#include <JPEGENC.h>
#include <SPI.h>
#include <SD.h>
SPIClass SD_SPI;

// GPIOs for the JC4827W543
#define SD_SCK 12
#define SD_MISO 13
#define SD_MOSI 11
#define SD_CS 10

BB_SPI_LCD lcd;
JPEGENC jpg;
JPEGENCODE jpe;
uint16_t u16Temp[320 * 16]; // hold 16 lines for capturing the MCUs (16x16) and sending to the display

int ProcessBMP(const uint8_t *pBMP, int iSize)
{
    int x, y, w, h, k, xoff, yoff, bits, offset;
    uint16_t *s;
    int rc, pitch, bytewidth;
    int iDataSize, iOutputSize, iDelta;
    uint8_t *pOutput;

    if (pBMP[0] != 'B' || pBMP[1] != 'M' || pBMP[14] < 0x28) {
        return 0;
    }
    w = *(int32_t *)&pBMP[18];
    h = *(int32_t *)&pBMP[22];
    bits = *(int16_t *)&pBMP[26] * *(int16_t *)&pBMP[28];
    if (bits != 16) { // only 16-bit images are supported
        return 0;
    }
    iOutputSize = 65536;
    pOutput = (uint8_t *)malloc(iOutputSize); // 64K should be plenty for this image
    offset = *(int32_t *)&pBMP[10]; // offset to bits
    bytewidth = (w * bits) >> 3;
    pitch = (bytewidth + 3) & 0xfffc; // DWORD aligned
    s = (uint16_t *)&pBMP[offset];
    iDelta = pitch;
    if (h > 0) {
        iDelta = -pitch;
        s = (uint16_t *)&pBMP[offset + (h-1) * pitch];
    } else {
        h = -h;
    }
    xoff = (lcd.width() - w)/2;
    yoff = (lcd.height() - h)/2;
    // Display and compress the BMP image at the same time
    lcd.setAddrWindow(xoff, yoff, w, h);
    rc = jpg.open(pOutput, iOutputSize);
    rc = jpg.encodeBegin(&jpe, w, h, JPEGE_PIXEL_RGB565, JPEGE_SUBSAMPLE_420, JPEGE_Q_HIGH);
    if (rc == JPEGE_SUCCESS) {
        for (y=0; y<h; y+=16) { // MCUs are 16 lines tall for 4:2:0 subsampling
            for (k=0; k<16; k++) {
                for (x=0; x<w; x++) {
                    u16Temp[(k*w)+x] = __builtin_bswap16(s[x]); // the LCD wants big-endian data
                } // for x
                lcd.pushPixels(&u16Temp[k*w], w); // push 1 line
                memcpy(&u16Temp[k*w], s, w*sizeof(uint16_t)); // JPEGENC expects little endian data
                s += iDelta/sizeof(uint16_t);
            } // for k
           for (x=0; x < w; x+= 16) {
              rc = jpg.addMCU(&jpe, (uint8_t *)&u16Temp[x], w * sizeof(uint16_t)); // encode the row of MCUs
           } // for each MCU
        } // for y
        iDataSize = jpg.close();
        lcd.setCursor(0,h+yoff);
        lcd.printf("out size = %d", iDataSize);
        // Write the JPEG file to the SD card root dir
        File file;
        file = SD.open("/out.jpg", FILE_WRITE, true); // overwrite if it exists
        if (file) {
            file.write(pOutput, iDataSize);
            file.close();
            lcd.print(", write succeeded!");
        } else {
            lcd.print(", write failed");
        }
        free(pOutput);
    } // init success
    return 1;
} /* ShowBMP() */

void setup()
{
  lcd.begin(DISPLAY_CYD_543); // JC4827W543 ESP32-S3 + 480x272 QSPI LCD
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_GREEN);
  lcd.setFont(FONT_12x16);
  lcd.println("JPEG Compression example");
  SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI, -1);
  if (!SD.begin(SD_CS, SD_SPI, 8000000)) { // Faster than 10MHz seems to fail on the CYDs
//  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, -1);
//  if (!SD.begin(SD_CS)) {
    lcd.setTextColor(TFT_RED);
    lcd.println("Card Mount Failed");
    while (1) {};
  } else {
    lcd.println("Card Mount Succeeded");
  }

  if (!ProcessBMP(rgb565, sizeof(rgb565))) { // display the image on the LCD
    lcd.println("Error reading BMP file");
    while (1) {};
  }
} /* setup() */

void loop()
{

} /* loop() */
