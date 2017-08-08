#include <LPD8806multi.h>

#include <Ethernet.h>
#include "SPI.h"

// Multi strip uses multiple clock and data pins in a fixed time (see LPD8806Multi.cpp)
LPD8806multi strip = LPD8806multi();

// Ethernet config
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
int port = 1500;
EthernetServer server = EthernetServer(port);

long nrcvd;

void setup() {
  int i;

  strip.init();
  // Update the strip, to start they are all 'off'
  show();
  // Turn on one for power indication
  strip.setPixelColor(0, strip.Color(0, 127, 0));
  show();


  // Setup serial port
  Serial.begin(115200);

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
  show();

  // Setup ethernet
  // NOTE: 3/11/13 - these were set to 192.168.3.* for use with switch, etc
  IPAddress ip(192, 168, 0, 70);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);

  Ethernet.begin(mac, ip, gateway, subnet);
  //Ethernet.begin(mac);
  strip.setPixelColor(2, strip.Color(0, 127, 0));

  //Ethernet.begin(mac, ip, gateway, subnet); //for manual setup
  server.begin();
  strip.setPixelColor(3, strip.Color(0, 127, 0));

  Serial.print("Listening at ");
  Serial.print(Ethernet.localIP());
  Serial.print(" port: ");
  Serial.println(port);

  strip.setPixelColor(4, strip.Color(0, 127, 0));
  strip.setPixelColor(0, strip.Color(127, 0, 0));
  show();


  nrcvd = 0;

  Serial.print("Have ");
  Serial.print((int)strip.numPixels());
  Serial.println(" leds");
}


// Retrieve cmdlen bytes into buffer, return -1 if EOF, otherwise wait as needed
int getcmd(EthernetClient *client, uint8_t *cmdbuf, int cmdlen) {
  while (cmdlen > 0) {
    int nr = client->read(cmdbuf, cmdlen);
    if (nr == 0) {
      Serial.println("Error reading cmd");
      return -1;
    }
    if (nr > 0) {
      cmdbuf += nr;
      cmdlen -= nr;
    }
  }
  return 0;   // OK
}

uint8_t ecSocket(EthernetClient *client)
{
  // KLUDGE-Read value of sock from private data in EthernetClient
  return ((uint8_t *)client)[2];
}

void loop() {
  EthernetClient client = server.available();

  if (client) {
    static unsigned long lasttime = micros(), pause = 0;
    uint8_t cmdbuf[20];
    while (client.available() != 0) {
      unsigned int r, g, b, index, index2, i1, i1b, i2, i2b;
      int id;
      // Get a command from host
      int cmd = client.read();
      Serial.print((char)cmd);
      nrcvd++;
      switch (cmd) {
        case 'S':  // Set
          // Set an LED color
          while (client.available() < 5 && client.connected())
            ;
          index = client.read();
          index2 = client.read();
          index = index2 * 256 + index;

          r = client.read();
          g = client.read();
          b = client.read();
          //Serial.print(index);
          //Serial.print(": ");
          //Serial.print(r);
          //Serial.print(",");
          //Serial.print(g);
          //Serial.print(",");
          //Serial.println(b);
          nrcvd += 5;
          strip.setPixelColor(index, strip.Color(r, g, b));
          break;
        case 'F':	// Fast set mode F start_low,start_high,cnt,grbgrbgrb...; sets up to 255 LEDs in one command
          // Note order is GRB, not RGB
          if (getcmd(&client, cmdbuf, 3) >= 0) {
            uint16_t index = cmdbuf[1] * 256 + cmdbuf[0];
            int cnt = cmdbuf[2] * 3;
            uint8_t *p = strip.getPixelAddr(index);
            //Serial.print("F: ");
            //Serial.print(index);
            //Serial.print(",");
            //Serial.println(cnt);
            nrcvd += cnt + 3;
            //uint16_t ngrp=0;
            while (cnt > 0) {
              int nread = client.read(p, cnt);
              //Serial.print("read ");
              //Serial.print(nread);
              //Serial.print("/");
              //Serial.println(cnt);
              if (nread == cnt)
                // Common case, fastest
                break;
              else if (nread == 0) { // EOF
                Serial.println("disconnect");
                return;
              } else if (nread < 0) { // Nothing available
                Serial.println("Empty");
                nread = 0;
              }
              cnt = cnt - nread;
              p += nread;
              //ngrp++;
            }

            //Serial.println("done");
            //Serial.print(ngrp);
            //Serial.println(" groups");
          } else
            Serial.println("getcmd failed\n");
          break;
        case 'E': // Echo (for testing)  - E [bytes to arduino] [ bytes from arduino] data...
          {
            uint8_t ebuf[256];
            uint8_t n[2];
            if (getcmd(&client, n, 2) < 0)
              Serial.println("Echo read cmd failure");
            if (getcmd(&client, ebuf, n[0]) < 0)
              Serial.println("Echo read data failure");
            if (client.write(ebuf, n[1]) != n[1])
              Serial.println("Echo write failure");
          }
        case 'G':  // Go
          if (pause > 0 && micros() < lasttime + pause) {
            Serial.print("Pausing for ");
            Serial.print(pause - (micros() - lasttime));
            Serial.print(" usec...");
            while (micros() < lasttime + pause)
              ;
            Serial.println("done");
          }
          lasttime = micros();
          show();
          pause = 0;
          break;
        case 'P':  // Pause
          // Hold LEDs for at least this much time since last change (in msec.)
          while (client.available() < 1 && client.connected())
            ;
          pause += ((unsigned long)client.read()) * 1000;
          // Serial.println(pause);
          nrcvd = nrcvd + 1;
          break;
        case 'A':  // All
          // Set all LEDs to a color
          while (client.available() < 3 && client.connected())
            ;
          r = client.read();
          g = client.read();
          b = client.read();
          nrcvd += 3;
          for (index = 0; index < strip.numPixels(); index++)
            strip.setPixelColor(index, strip.Color(r, g, b));
          break;
        case 'R':  // Range
          // Set a range of LEDs to a color
          while (client.available() < 7 && client.connected())
            ;
          i1 = client.read();
          i1b = client.read();
          i1 = i1 + 256 * i1b;
          i2 = client.read();
          i2b = client.read();
          i2 = i2 + 256 * i2b;
          r = client.read();
          g = client.read();
          b = client.read();
          nrcvd += 7;
          for (index = i1; index <= i2; index++)
            strip.setPixelColor(index, strip.Color(r, g, b));
          break;
        case 'N':   // Num bytes received
          // Number of bytes received
          client.write('N');
          client.write((nrcvd >> 24) & 0xff);
          client.write((nrcvd >> 16) & 0xff);
          client.write((nrcvd >> 8) & 0xff);
          client.write(nrcvd & 0xff);
          nrcvd = 0;
          break;
        case 'V':   // Verify (Ack)
          {
            // Note: checking client.available() sometimes gives a count of 1, but a subsequent client.read() returns -1
            // There may be a race in WD ethernet code that causes subsequent arrival of data (was seeing 769 bytes when rechecking)
            // to temporarily make it look the queue is empty again
            int cnt = 1;
            while ((id = client.read()) < 0) {
              if (!client.connected()) {
                Serial.println("Client disconnect during V command");
                break;
              }
              cnt = cnt + 1;
              if (cnt >= 10000) {
                Serial.print('#');
                cnt = 0;
              }
            }
            //Serial.print('+');
            nrcvd++;
            client.write('A');  // ACK
            client.write(id);
            //Serial.print('@');
            //Serial.print('V');
            //Serial.println(id);
          }
          break;
      }
    }
    Serial.println(".");
  }
}

void show() {
  strip.show();
}
