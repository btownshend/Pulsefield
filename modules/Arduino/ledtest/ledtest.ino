#include <LPD8806multi.h>

#include "SPI.h"

// Multi strip uses multiple clock and data pins in a fixed time (see LPD8806Multi.cpp)
LPD8806multi strip = LPD8806multi();


void setup() {
  int i;


  // Setup serial port
  Serial.begin(115200);
  
  strip.init();
  // Update the strip, to start they are all 'off'
  strip.show();
  // Turn on one for power indication
  strip.setPixelColor(0, strip.Color(0, 127, 0));
  strip.show();


  if (1) {
    // If using external memory, adjust heap end up
    Serial.print("Heap start = 0x");
    Serial.print((int)__malloc_heap_start, HEX);
    Serial.print(", heap end = 0x");
    Serial.println((int)__malloc_heap_end, HEX);
    // __malloc_heap_start = (char *)0x1100;

    // Check max malloc size
    for (i = 1; i < 50000; i += 10) {
      int *x = (int *)malloc(i);
      free(x);
      if (x == NULL) {
        Serial.print("Max alloc = ");
        Serial.println(i - 1);
        break;
      }
    }

    for (i = 32; i <= 53; i++) {
      Serial.print("Pin ");
      Serial.print(i);
      Serial.print(": port=");
      Serial.print(digitalPinToPort(i));
      Serial.print("@");
      Serial.print((int)portOutputRegister(digitalPinToPort(i)), HEX);
      Serial.print(", bitmask=0x");
      Serial.println(digitalPinToBitMask(i), HEX);
    }
  }

  strip.setPixelColor(1, strip.Color(0, 127, 0));
  strip.show();

  strip.setPixelColor(2, strip.Color(0, 127, 0));

  strip.setPixelColor(3, strip.Color(0, 127, 0));

  strip.setPixelColor(4, strip.Color(0, 127, 0));
  strip.setPixelColor(0, strip.Color(127, 0, 0));
  strip.show();

  Serial.print("Have ");
  Serial.print((int)strip.numPixels());
  Serial.println(" leds");
  //for (int index=0;index<strip.numPixels();index++) {
  //  strip.setPixelColor(index, strip.Color(index%128,(index*3)%128, (index*7)%128));
  //}
}


// Periods in msecs
float baseperiod = 3;
float rperiod = baseperiod;
float gperiod = baseperiod + 2;
float bperiod = baseperiod + 3;
float rspacing = 30;
float gspacing = 30;
float bspacing = 30;
float pi = 3.14159;
int offset = 1;
long int lastmilli=0;
int count=0;

void loop() {
  int r = 63 + 63 * cos(2 * pi * (millis() / rperiod / 1000 ));
  int g = 63 + 63 * cos(2 * pi * (millis() / gperiod / 1000 ));
  int b = 63 + 63 * cos(2 * pi * (millis() / bperiod / 1000 ));

  for (int index = 0; index < strip.numPixels() - offset; index++) {
    strip.setPixelColor(index, strip.getPixelColor(index + offset));
  }
  for (int index = 1; index <= offset; index++)
    strip.setPixelColor(strip.numPixels() - offset, strip.Color(r, g, b));
  //delay(100);
  strip.show();
  if (count>30) {
  Serial.println((millis()-lastmilli)*1.0/count);
  count=0;
  lastmilli=millis();
  }
  count++;
}
