#include <LPD8806multi.h>

#include <Ethernet.h>
#include "SPI.h"

// Multi strip uses multiple clock and data pins in a fixed time (see LPD8806Multi.cpp)
LPD8806multi strip = LPD8806multi();

// Ethernet config
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
int port=1500;
EthernetServer server = EthernetServer(port);

long nrcvd;

void setup() {
    int i;
    
    // Setup serial port
    Serial.begin(115200);
    
    if (1) {
	// Check max malloc size
	for (i=1;i<50000;i+=10) {
	    int *x=(int *)malloc(i);
	    free(x);
	    if (x==NULL) {
		Serial.print("Max alloc = ");
		Serial.println(i-1);
		break;
	    }
	}

	for (i=32;i<=53;i++) {
	    Serial.print("Pin ");
	    Serial.print(i);
	    Serial.print(": port=");
	    Serial.print(digitalPinToPort(i));
            Serial.print("@");
            Serial.print((int)portOutputRegister(digitalPinToPort(i)),HEX);
	    Serial.print(", bitmask=0x");
	    Serial.println(digitalPinToBitMask(i),HEX);
	}
    }

    // Setup ethernet
    Ethernet.begin(mac);
    //Ethernet.begin(mac, ip, gateway, subnet); //for manual setup
    server.begin();
    Serial.print("Listening at ");
    Serial.print(Ethernet.localIP());
    Serial.print(" port: ");
    Serial.println(port);
  
  
    strip.init();
    // Update the strip, to start they are all 'off'
    show();
    // Turn on one for power indication
    strip.setPixelColor(0, strip.Color(0,127,0));
    show();

    nrcvd=0;

    Serial.print("Have ");
    Serial.print((int)strip.numPixels());
    Serial.println(" leds");
}


// Retrieve cmdlen bytes into buffer, return -1 if EOF, otherwise wait as needed
int getcmd(EthernetClient *client, uint8_t *cmdbuf,int cmdlen) {
    while (cmdlen>0) {
        int nr=client->read(cmdbuf,cmdlen);
	if (nr==0) {
	   Serial.println("Error reading cmd");
	   return -1;
	}
	if (nr>0) {
	   cmdbuf+=nr;
	   cmdlen-=nr;
       }
    }
    return 0;   // OK
}

void loop() {
    EthernetClient client = server.available();

    if (client) {
	static unsigned long lasttime=micros(), pause=0;
        uint8_t cmdbuf[20];
	while (client.available() != 0) {
	    unsigned int id,r,g,b,index,index2,i1,i1b,i2,i2b;
	    // Get a command from host 
	    int cmd=client.read();
	    //Serial.print("Got command: ");
	    //Serial.println(cmd);
	    nrcvd++;
	    switch(cmd) {
	    case 'S':  // Set
		// Set an LED color
		while (client.available()<5 && client.connected())
		    ;
		index=client.read();
		index2=client.read();
		index=index2*256+index;

		r=client.read();
		g=client.read();
		b=client.read();
                //Serial.print(index);
		//Serial.print(": ");
		//Serial.print(r);
		//Serial.print(",");
		//Serial.print(g);
		//Serial.print(",");
		//Serial.println(b);
		nrcvd+=5;
		strip.setPixelColor(index, strip.Color(r,g,b));
		break;
	    case 'F':	// Fast set mode F start_low,start_high,cnt,grbgrbgrb...; sets up to 255 LEDs in one command
		// Note order is GRB, not RGB
    		if (getcmd(&client, cmdbuf, 3) >= 0) {
		    uint16_t index=cmdbuf[1]*256+cmdbuf[0];
		    int cnt=cmdbuf[2]*3;
		    uint8_t *p=strip.getPixelAddr(index);
		    //Serial.print("F: ");
		    //Serial.print(index);
		    //Serial.print(",");
		    //Serial.println(cnt);
		    nrcvd+=cnt+3;
		    //uint16_t ngrp=0;
		    while (cnt>0) {
			int nread=client.read(p,cnt);
			//Serial.print("read ");
			//Serial.print(nread);
			//Serial.print("/");
			//Serial.println(cnt);
			if (nread==cnt)
			    // Common case, fastest
			    break;
			else if (nread==0) { // EOF
			    Serial.println("disconnect");
			    return;
			} else if (nread<0) { // Nothing available
			    Serial.println("Empty");
			    nread=0;
			}
			cnt=cnt-nread;
			p+=nread;
			//ngrp++;
		    }
		
		    //Serial.println("done");
		    //Serial.print(ngrp);
		    //Serial.println(" groups");
		} else
		     Serial.println("getcmd failed\n");
		break;
	    case 'G':  // Go
		if (pause>0 && micros()<lasttime+pause) {
		    Serial.print("Pausing for ");
		    Serial.println(pause-(micros()-lasttime));
		    Serial.print(" usec...");
		    while (micros()<lasttime+pause)
			;
		    //clSerial.println("done");
		}
		lasttime=micros();
		show();
		pause=0;
		break;
	    case 'P':  // Pause
		// Hold LEDs for at least this much time since last change (in msec.)
		while (client.available()<1 && client.connected())
		    ;
		pause+=((unsigned long)client.read())*1000;
		// Serial.println(pause);
		nrcvd=nrcvd+1;
		break;
	    case 'A':  // All
		// Set all LEDs to a color
		while (client.available()<3 && client.connected())
		    ;
		r=client.read();
		g=client.read();
		b=client.read();
		nrcvd+=3;
		for (index=0;index<strip.numPixels();index++)
		    strip.setPixelColor(index, strip.Color(r,g,b));
		break;
	    case 'R':  // Range
		// Set a range of LEDs to a color
		while (client.available()<7 && client.connected())
		    ;
		i1=client.read();
		i1b=client.read(); 
		i1=i1+256*i1b;
		i2=client.read();
		i2b=client.read(); 
		i2=i2+256*i2b;
		r=client.read();
		g=client.read();
		b=client.read();
		nrcvd+=7;
		for (index=i1;index<=i2;index++)
		    strip.setPixelColor(index, strip.Color(r,g,b));
		break;
	    case 'N':   // Num bytes received
		// Number of bytes received
		client.write('N');
		client.write((nrcvd>>24)&0xff);
		client.write((nrcvd>>16)&0xff);
		client.write((nrcvd>>8)&0xff);
		client.write(nrcvd&0xff);
		nrcvd=0;
		break;
	    case 'V':   // Verify (Ack)
		while (client.available()<1 && client.connected())
		    ;
		id=client.read();
		nrcvd++;
		client.write('A');  // ACK
		client.write(id);
		break;
	    } 
	}
    }
}

void show() {
    strip.show();
}
