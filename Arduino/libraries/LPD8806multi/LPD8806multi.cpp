#include "SPI.h"
#include "LPD8806multi.h"

// Arduino library to control LPD8806multi-based RGB LED Strips
// (c) Adafruit industries
// MIT license
#define NSTRIPS 7
static const int nstrips=NSTRIPS;

#if NSTRIPS==8
static const int nports=3;
// Masks for direct control of pins
static const uint8_t datapins[nstrips]={49,47,45,43,37,35,33,41};
static const uint8_t clkpins[nstrips]= {48,46,44,42,36,34,32,40};
static const uint8_t ports[nstrips]={0,0,0,0,1,1,1,2};
TODO -- havent fully modified code to support 8 strips
#else
static const int nports=2;
// Masks for direct control of pins
static const uint8_t datapins[nstrips]={49,47,45,43,37,35,33};
static const uint8_t clkpins[nstrips]= {48,46,44,42,36,34,32};
static const uint8_t ports[nstrips]={0,0,0,0,1,1,1};
static const uint8_t clkpinmaskperport[nports] = {0xaa,0x2a};
static const uint8_t datapinmaskperport[nports]={0x55,0x1f};
static const uint8_t datapinmask[nstrips]={0x1,0x4,0x10,0x40,0x1,0x4,0x10};
#endif

// I/O register address
static volatile uint8_t *portAddrs[nports];

// Constructor with compile-time fixed strip config
LPD8806multi::LPD8806multi() {
    Serial.begin(115200);
    Serial.println("LPD8806multi()");
}

void LPD8806multi::init() {
    alloc(160*nstrips);
    // Enable pins as ouput
    //    for (int i=0;i<nports;i++) {
    //	clkpinmaskperport[i]=0;
    //datapinmaskperport[i]=0;
    //}
    for (int i=0;i<nstrips;i++) {
	pinMode(datapins[i], OUTPUT);
	pinMode(clkpins[i], OUTPUT);
	//datapinmask[i]=digitalPinToBitMask(datapins[i]);
	//	datapinmaskperport[ports[i]]|=datapinmask[i];
	//	clkpinmaskperport[ports[i]]|=digitalPinToBitMask(clkpins[i]);
	portAddrs[ports[i]]=portOutputRegister(digitalPinToPort(datapins[i]));  // Assume clk port is same as data port;  sets multiple times, but should always be same value
    }
    Serial.println("setup outputs");
    writeLatch();

    Serial.print("data port addresses: ");
    for (int i=0;i<nstrips;i++) {
	Serial.print(ports[i]);
	Serial.print(": ");
	Serial.print((int)portOutputRegister(digitalPinToPort(datapins[i])),HEX);
	Serial.print(",");
    }
    Serial.println("");

    Serial.print("clk port addresses: ");
    for (int i=0;i<nstrips;i++) {
	Serial.print(ports[i]);
	Serial.print(": ");
	Serial.print((int)portOutputRegister(digitalPinToPort(clkpins[i])),HEX);
	Serial.print(",");
    }
    Serial.println("");
}

// Allocate 3 bytes per pixel, init to RGB 'off' state:
void LPD8806multi::alloc(uint16_t n) {
    // Allocate 3 bytes per pixel:
    if (NULL != (pixels = (uint8_t *)malloc(n * 3))) {
	memset(pixels, 0x80, n * 3); // Init to RGB 'off' state
	numLEDs = n;
    } else
	numLEDs = 0;
    begun =  false;
    pause = 0;
}

uint16_t LPD8806multi::numPixels(void) {
    return numLEDs;
}

// Issue latch of appropriate length; pass # LEDs, *not* latch length
void LPD8806multi::writeLatch() {
    // Latch length varies with the number of LEDs:
    const uint16_t nled=160;
    const uint16_t n=((nled + 63) / 64) * 3;
    
    // Set all the data low
    for (int i=0;i<nports;i++)
	*portAddrs[i] &= ~datapinmaskperport[i]; // Data is held low throughout

    for(uint16_t i = 8 * n; i>0; i--) {
	*portAddrs[0] |=  clkpinmaskperport[0];
	*portAddrs[0] &= ~clkpinmaskperport[0];
	*portAddrs[1] |=  clkpinmaskperport[1];
	*portAddrs[1] &= ~clkpinmaskperport[1];
    }
}

// This is how data is pushed to the strip.  Unfortunately, the company
// that makes the chip didnt release the  protocol document or you need
// to sign an NDA or something stupid like that, but we reverse engineered
// this from a strip controller and it seems to work very nicely!
void LPD8806multi::show(void) {
    uint16_t i, nl3 = 160 * 3; // 3 bytes per LED (all strips done in parallel)
    //  Serial.print("Datapinmask=0x");
    //  Serial.println(datapinmask,HEX);
  
    // write 24 bits per pixel
    uint8_t *pptr=&pixels[0];
    for (i=0; i<nl3; i++ ) {
	uint8_t p0=*pptr;
	uint8_t p1=pptr[160*3];
	uint8_t p2=pptr[160*6];
	uint8_t p3=pptr[160*9];
	uint8_t p4=pptr[160*12];
	uint8_t p5=pptr[160*15];
	uint8_t p6=pptr[160*18];
	for (uint8_t bit=0x80; bit; bit >>= 1) {
	    *portAddrs[0] = ((p0&0x80)?datapinmask[0]:0) |
		((p1 & 0x80)?datapinmask[1]:0) |
		((p2 & 0x80)?datapinmask[2]:0) |
		((p3 & 0x80)?datapinmask[3]:0) ;
	    // If the clock pin is raised at the same time, then clock skew on long cables can cause bad data latching
	    //		clkpinmaskperport[0];
	    *portAddrs[0] |= clkpinmaskperport[0];
	    //	    *portAddrs[0] &= ~clkpinmaskperport[0];   // Don't need to do this -- will be lowered with next data load
	    p0<<=1;
	    p1<<=1;
	    p2<<=1;
	    p3<<=1;

	    *portAddrs[1]= ((p4 & 0x80)?datapinmask[4]:0) |
		((p5 & 0x80)?datapinmask[5]:0) |
		((p6 & 0x80)?datapinmask[6]:0)  ;
	    // clkpinmaskperport[1];
	    *portAddrs[1] |= clkpinmaskperport[1];
	    //*portAddrs[1] &= ~clkpinmaskperport[1];   // Don't need to -- will be lowered with next data load
	    p4<<=1;
	    p5<<=1;
	    p6<<=1;
#if NSTRIPS>7
	    x=(*portAddrs[2] & ~datapinmaskperport[2]);
	    if (pptr[160*21] & bit) x |=  datapinmask[7];
	    *portAddrs[2] =  x;
	    *portAddrs[2] |= clkpinmaskperport[2];
	    *portAddrs[2] &= ~clkpinmaskperport[2];
#endif
	}
	pptr++;
    }
    
    writeLatch(); // Write latch at end of data

    // We need to have a delay here, a few ms seems to do the job
    // shorter may be OK as well - need to experiment :(
    // delay(pause);
}

// Convert separate R,G,B into combined 32-bit GRB color:
uint32_t LPD8806multi::Color(byte r, byte g, byte b) {
    return 0x808080 | ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
}

// Set pixel color from separate 7-bit R, G, B components:
void LPD8806multi::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
    if (n < numLEDs) { // Arrays are 0-indexed, thus NOT '<='
	uint8_t *p = &pixels[n * 3];
	*p++ = g | 0x80; // LPD8806multi color order is GRB,
	*p++ = r | 0x80; // not the more common RGB,
	*p++ = b | 0x80; // so the order here is intentional; don't "fix"
    }
}

// Set pixel color from 'packed' 32-bit RGB value:
void LPD8806multi::setPixelColor(uint16_t n, uint32_t c) {
    if (n < numLEDs) { // Arrays are 0-indexed, thus NOT '<='
	uint8_t *p = &pixels[n * 3];
	*p++ = (c >> 16) | 0x80;
	*p++ = (c >>  8) | 0x80;
	*p++ =  c        | 0x80;
    }
}

// Query color from previously-set pixel (returns packed 32-bit GRB value)
uint32_t LPD8806multi::getPixelColor(uint16_t n) {
    if (n < numLEDs) {
	uint16_t ofs = n * 3;
	return ((uint32_t)((uint32_t)pixels[ofs    ] << 16) |
		(uint32_t)((uint32_t)pixels[ofs + 1] <<  8) |
		(uint32_t)pixels[ofs + 2]) & 0x7f7f7f;
    }

    return 0; // Pixel # is out of bounds
}
