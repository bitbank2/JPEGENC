//
// JPEG encoder performance test
// written by Larry Bank
//
// example sketch to dynamically generate a JPEG image
// and throw away the output data to time the processing only
//
#include <JPEGENC.h>

JPEG jpg;

//
// Callback functions needed by the unzipLIB to access a file system
// The library has built-in code for memory-to-memory transfers, but needs
// these callback functions to allow using other storage media
//
void * myOpen(const char *filename) {
  (void)filename;
  return (void *)1; // non-NULL value
}
void myClose(JPEGFILE *p) {
  (void)p;
}
int32_t myRead(JPEGFILE *p, uint8_t *buffer, int32_t length) {
  (void)p;
  (void)buffer;
  return length;
}

int32_t myWrite(JPEGFILE *p, uint8_t *buffer, int32_t length) {
  (void)p;
  (void)buffer;
  return length;
}

int32_t mySeek(JPEGFILE *p, int32_t position) {
  (void)p;
  return position;
}


void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("JPEG encoder performance test");
} /* setup() */

void loop() {
const int iWidth = 1024, iHeight = 1024;
int i, iMCUCount, rc, iDataSize;
JPEGENCODE jpe;
uint8_t ucMCU[64];
long lTime;

  rc = jpg.open("/TEST.JPG", myOpen, myClose, myRead, myWrite, mySeek);
  if (rc == JPEG_SUCCESS) {
      Serial.println("JPEG file opened successfully");
      lTime = micros();
      rc = jpg.encodeBegin(&jpe, iWidth, iHeight, JPEG_PIXEL_GRAYSCALE, JPEG_SUBSAMPLE_444, JPEG_Q_HIGH);
      if (rc == JPEG_SUCCESS) {
          memset(ucMCU, 0, sizeof(ucMCU));
          
          iMCUCount = ((iWidth + jpe.cx-1)/ jpe.cx) * ((iHeight + jpe.cy-1) / jpe.cy);
          for (i=0; i<iMCUCount && rc == JPEG_SUCCESS; i++) {
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
          lTime = micros() - lTime;
          Serial.print("Output file size = ");
          Serial.println(iDataSize, DEC);
          Serial.print("Encoding time = ");
          Serial.print((int)lTime, DEC);
          Serial.println("us");
          delay(5000);
      }
  } // opened successfully
}
