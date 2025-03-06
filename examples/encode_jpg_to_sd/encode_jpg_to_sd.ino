//
// Simple JPEG encoder test
// written by Larry Bank
//
// example sketch to dynamically generate a JPEG image
// and write it to a file on a uSD card
//
#include <JPEGENC.h>
#include <SD.h>

JPEGENC jpg;
static File myfile;

//
// Callback functions needed by the unzipLIB to access a file system
// The library has built-in code for memory-to-memory transfers, but needs
// these callback functions to allow using other storage media
//
void * myOpen(const char *filename) {
  myfile = SD.open(filename, FILE_WRITE);
  return (void *)&myfile;
}
void myClose(JPEGE_FILE *p) {
  File *f = (File *)p->fHandle;
  if (f) f->close();
}
int32_t myRead(JPEGE_FILE *p, uint8_t *buffer, int32_t length) {
  File *f = (File *)p->fHandle;
  return f->read(buffer, length);
}

int32_t myWrite(JPEGE_FILE *p, uint8_t *buffer, int32_t length) {
  File *f = (File *)p->fHandle;
  return f->write(buffer, length);
}

int32_t mySeek(JPEGE_FILE *p, int32_t position) {
  File *f = (File *)p->fHandle;
  return f->seek(position);
}


void setup() {
  Serial.begin(115200);
  delay(2000);
  // PyPortal SD CS = GPIO 32
  while (!SD.begin(32)) { // change this to the appropriate value for your setup
    Serial.println("Unable to access SD Card");
    delay(1000);
  }
  Serial.println("SD card opened");
} /* setup() */

void loop() {
const int iWidth = 1024, iHeight = 1024;
int i, iMCUCount, rc, iDataSize;
JPEGENCODE jpe;
uint8_t ucMCU[64];

  rc = jpg.open("/TEST.JPG", myOpen, myClose, myRead, myWrite, mySeek);
  if (rc == JPEGE_SUCCESS) {
      Serial.println("JPEG file opened successfully");
      rc = jpg.encodeBegin(&jpe, iWidth, iHeight, JPEGE_PIXEL_GRAYSCALE, JPEGE_SUBSAMPLE_444, JPEGE_Q_HIGH);
      if (rc == JPEGE_SUCCESS) {
          memset(ucMCU, 0, sizeof(ucMCU));
          
          iMCUCount = ((iWidth + jpe.cx-1)/ jpe.cx) * ((iHeight + jpe.cy-1) / jpe.cy);
          for (i=0; i<iMCUCount && rc == JPEGE_SUCCESS; i++) {
              // Send two types of MCUs (a simple diagonal line, and a blank box)
              if (i & 1) { // odd MCUs
                for (int j=0; j<8; j++)
                  ucMCU[j*8+j] = 192; // create an image composed of diagonal white lines
              } else { // even MCUs
                for (int j=0; j<8; j++)
                  ucMCU[j*8+j] = 0; // blank
              }
              rc = jpg.addMCU(&jpe, ucMCU, 8);
          }
          iDataSize = jpg.close();
          Serial.print("Output file size = ");
          Serial.println(iDataSize, DEC);
          delay(5000);
      }
  } // opened successfully
}
