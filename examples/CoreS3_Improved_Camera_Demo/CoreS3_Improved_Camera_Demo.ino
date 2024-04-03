//
// Updated camera demo for the CoreS3
// written by Larry Bank
//
// The GC0308 640x480 camera only supports
// uncompressed output (not JPEG). To create
// JPEG images (e.g. for transmission over WiFi)
// We must use a software JPEG encoder. The
// JPEG codec supplied by Espressif:
// https://github.com/espressif/esp32-camera/blob/master/conversions/to_jpg.cpp
// converts incoming pixels to RGB for compression by
// The problem is that JPEG uses the YCbCr colorspace, which is nearly
// identical to the YUV colorspace (which the camera can supply).
// A new problem - if you give Espressif's library YUV, it
// will convert it first to RGB, then YCbCr (2 color conversion steps).
// This demo is to show usage of my latest JPEGENC (v1.1.0) encoder which can accept
// YUV422 input data and compress it into a JPEG file much more quickly.
// 
#include "M5CoreS3.h"
#include "esp_camera.h"
#include <JPEGENC.h>

#define CONVERT_TO_JPEG
JPEGENC jpgenc;

void setup() {
    Serial.begin(115200);
    auto cfg = M5.config();
    CoreS3.begin(cfg);
    CoreS3.Display.setTextColor(GREEN);
    CoreS3.Display.setTextDatum(middle_center);
    CoreS3.Display.setFont(&fonts::Orbitron_Light_24);
    CoreS3.Display.setTextSize(1);

    if (!CoreS3.Camera.begin()) {
        CoreS3.Display.drawString("Camera Init Fail",
                                  CoreS3.Display.width() / 2,
                                  CoreS3.Display.height() / 2);
    }
    CoreS3.Display.drawString("Camera Init Success", CoreS3.Display.width() / 2,
                              CoreS3.Display.height() / 2);

    CoreS3.Camera.sensor->set_framesize(CoreS3.Camera.sensor, FRAMESIZE_QVGA);
}

void loop() {
  long l1, l2;
  JPEGENCODE enc;

    // Set Espressif's fastest input pixel format (RGB565)
    CoreS3.Camera.sensor->set_pixformat(CoreS3.Camera.sensor, PIXFORMAT_RGB565);
    if (CoreS3.Camera.get()) {
#ifdef CONVERT_TO_JPEG
        uint8_t *out_jpg   = NULL;
        size_t out_jpg_len1 = 0, out_jpg_len2;
        l1 = millis();
        frame2jpg(CoreS3.Camera.fb, 88, &out_jpg, &out_jpg_len1);
        l1 = millis() - l1;
        CoreS3.Display.drawJpg(out_jpg, out_jpg_len1, 0, 0,
                               CoreS3.Display.width(), CoreS3.Display.height());
        free(out_jpg);
        
        // Set JPEGENC's fastest input pixel format (YUV422)
        CoreS3.Camera.sensor->set_pixformat(CoreS3.Camera.sensor, PIXFORMAT_YUV422);
        l2 = millis();
        out_jpg = (uint8_t *)ps_malloc(65536);
        jpgenc.open(out_jpg, 65536);
        jpgenc.encodeBegin(&enc, CoreS3.Camera.fb->width, CoreS3.Camera.fb->height, JPEGE_PIXEL_YUV422, JPEGE_SUBSAMPLE_420, JPEGE_Q_MED);
        jpgenc.addFrame(&enc, CoreS3.Camera.fb->buf, CoreS3.Camera.fb->width * 2);
        out_jpg_len2 = jpgenc.close();
        l2 = millis() - l2;
        free(out_jpg);
        Serial.printf("frame2jpg: %dms %d bytes, JPEGENC: %dms %d bytes\n", (int)l1, out_jpg_len1, (int)l2, out_jpg_len2);
#else
        CoreS3.Display.pushImage(0, 0, CoreS3.Display.width(),
                                 CoreS3.Display.height(),
                                 (uint16_t *)CoreS3.Camera.fb->buf);
#endif

        CoreS3.Camera.free();
    }
}
