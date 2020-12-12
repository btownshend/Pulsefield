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
unsigned long lastReceived;   // Time in millis() of last received command

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

  lastReceived = millis(); // Time of last received byte

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

String fbsettings;

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
      lastReceived = millis(); // Time of last received byte
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
          // All of the data MUST have the high bit set or the data won't latch into the strip
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
                //Serial.println("Empty");
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
            Serial.println(id);
          }
          break;
      case '$':
	  // Set fallback mode and parameters
	  fbsettings="";
	  for (int i=0;i<80;i++) {
	      unsigned int x=client.read();
	      nrcvd++;
	      if (x==0 || x=='\n' || x=='\r')
		  break;
	      fbsettings+=char(x);
	  }
	  lastReceived=0;   // Jump immediately to fallback
	  break;
      }
    }
  }
  if (millis() - lastReceived > 5000)
    // No ethernet commands
    fallback();
//  else
//    Serial.print(".");
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
long int lastmilli = 0;
int count = 0;

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String getSetting(int index)
{
    return getValue(fbsettings.substring(1),',',index);
}

int getIntSetting(int index, int def)
{
    String s=getSetting(index);
    if (s.length() == 0)
	return def;
    else
	return s.toInt();
}

float getFloatSetting(int index, float def)
{
    String s=getSetting(index);
    if (s.length() == 0)
	return def;
    else
	return s.toFloat();
}

void rainbow() {
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
}

void sparkle() {
    int r = 63 + 63 * cos(2 * pi * (millis()  / getFloatSetting(2,3) / 1000 ));
    int g = 63 + 63 * cos(2 * pi * (millis()  / getFloatSetting(3,4) / 1000 ));
    int b = 63 + 63 * cos(2 * pi * (millis()  / getFloatSetting(4,5) / 1000 ));
    const int nsparkle=min(getIntSetting(0,30),100);
    static int onpixels[100];
    const int offperiod=getIntSetting(1,40);
    int debug=getIntSetting(5,0);
    
    // Loop through current on pixels
    for (int i=0;i<nsparkle;i++) {
	if (random(0,offperiod) == 0) {
	    // Change this pixel
	    if (debug) {
		Serial.print(onpixels[i]);
		Serial.print(" off, ");
	    }
	    strip.setPixelColor(onpixels[i], strip.Color(0,0,0));  // Turn off current pixel
	    onpixels[i]=random(0,strip.numPixels());   // Pick a new one
	    strip.setPixelColor(onpixels[i], strip.Color(r, g, b));  // Turn it on
	    if (debug) {
		Serial.print(onpixels[i]);
		Serial.print("=");
		Serial.print(r); Serial.print(",");
		Serial.print(g); Serial.print(",");
		Serial.println(b);
	    }
	}
    }
  strip.show();
}

void fallback() {
    static int i=0;
    static unsigned long lasttime=millis();
    switch (fbsettings[0]) {
    case 'R':
	rainbow();
	break;
    case 'S':
    default:
	sparkle();
    }
    i=i+1;
    if (i>=100) {
	unsigned long now=millis();
	float freq=100/((now-lasttime)/1000.0);
	Serial.print("rate=");
	Serial.print(freq);
	Serial.print(", settings=");
	Serial.println(fbsettings);
	if (0) {
	    for (int j=0;j<10;j++) {
		int x = getIntSetting(j,-1);
		Serial.print("arg[");
		Serial.print(j);
		Serial.print("]=");
		Serial.println(x);
		if (x<0)
		    break;
	    }
	}

	i=0;
	lasttime=now;
    }
}

void show() {
  strip.show();
}
