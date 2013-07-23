/*--------------------------------------------------------------------
  Arduino library to control a wide variety of WS2811-based RGB LED
  devices such as Adafruit FLORA RGB Smart Pixels.  Currently handles
  400 and 800 KHz bitstreams on both 8 MHz and 16 MHz ATmega MCUs,
  with LEDs wired for RGB or GRB color order.  8 MHz MCUs provide
  output on PORTB and PORTD, while 16 MHz chips can handle most output
  pins (possible exception with some of the upper PORT registers on
  the Arduino Mega).

  WILL NOT COMPILE OR WORK ON ARDUINO DUE.  Uses inline assembly.

  Written by Phil Burgess / Paint Your Dragon for Adafruit Industries.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  --------------------------------------------------------------------
  This file is part of the Adafruit NeoPixel library.

  NeoPixel is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  NeoPixel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with NeoPixel.  If not, see
  <http://www.gnu.org/licenses/>.
  --------------------------------------------------------------------*/

#include "Neo8.h"

Adafruit_NeoPixel::Adafruit_NeoPixel(uint16_t n, uint8_t p, uint8_t t) {
  numBytes = n * 24;
  if((pixels = (uint8_t *)malloc(numBytes))) {
    memset(pixels, 0, numBytes);
    numLEDs = n;
    type    = t;
    firstpin = p;
    port    = portOutputRegister(digitalPinToPort(p));
    endTime = 0L;
  } else {
    numLEDs = 0;
  }
}

void Adafruit_NeoPixel::begin(void) {
    for (int p=firstpin;p<firstpin+8;p++) {
	pinMode(p, OUTPUT);
	digitalWrite(p, LOW);
    }
}


#ifdef __arm__
static inline void delayShort(uint32_t) __attribute__((always_inline, unused));
static inline void delayShort(uint32_t num)
{
        asm volatile(
                "L_%=_delayMicroseconds:"               "\n\t"
                "subs   %0, #1"                         "\n\t"
                "bne    L_%=_delayMicroseconds"         "\n"
//#if F_CPU == 48000000
                //"nop"					"\n\t"
//#endif
                : "+r" (num) :
        );
}
#endif // __arm__


void Adafruit_NeoPixel::show(void) {

  if(!numLEDs) return;

  volatile uint16_t
    i   = numBytes; // Loop counter
  volatile uint8_t
   *ptr = pixels,   // Pointer to next byte
    b   = *ptr++,   // Current byte value
    hi,             // PORT w/output bit set high
    lo;             // PORT w/output bit set low

  // Data latch = 50+ microsecond pause in the output stream.
  // Rather than put a delay at the end of the function, the ending
  // time is noted and the function will simply hold off (if needed)
  // on issuing the subsequent round of data until the latch time has
  // elapsed.  This allows the mainline code to start generating the
  // next frame of data rather than stalling for the latch.
  while((micros() - endTime) < 50L);
  // endTime is a private member (rather than global var) so that
  // mutliple instances on different pins can be quickly issued in
  // succession (each instance doesn't delay the next).

  // In order to make this code runtime-configurable to work with
  // any pin, SBI/CBI instructions are eschewed in favor of full
  // PORT writes via the OUT or ST instructions.  It relies on two
  // facts: that peripheral functions (such as PWM) take precedence
  // on output pins, so our PORT-wide writes won't interfere, and
  // that interrupts are globally disabled while data is being issued
  // to the LEDs, so no other code will be accessing the PORT.  The
  // code takes an initial 'snapshot' of the PORT state, computes
  // 'pin high' and 'pin low' values, and writes these back to the
  // PORT register as needed.

  cli(); // Disable interrupts; need 100% focus on instruction timing

  //  This block now handles the
  // common alternate case for either: 800 KHz pixels w/16 MHz CPU, or
  // 400 KHz pixels w/8 MHz CPU.  Instruction timing is the same.
    // Can use nested loop; no need for unrolling.  Very similar to
    // 16MHz/400KHz code above, but with fewer NOPs and different end.

    // 20 inst. clocks per bit: HHHHxxxxxxxxxxxxLLLL
    // ST instructions:         ^   ^           ^

    volatile uint8_t next, bit;

    hi   = 255;
    lo   = 0;
    next = lo;
    bit  = 8;

    asm volatile(
     "head20:\n\t"          // Clk  Pseudocode    (T =  0)
      "st   %a0, %1\n\t"    // 2    PORT = hi     (T =  2)
      "sbrc %2, 7\n\t"      // 1-2  if(b & 128)
       "mov  %4, %1\n\t"    // 0-1   next = hi    (T =  4)
      "st   %a0, %4\n\t"    // 2    PORT = next   (T =  6)
      "mov  %4, %5\n\t"     // 1    next = lo     (T =  7)
      "dec  %3\n\t"         // 1    bit--         (T =  8)
      "breq nextbyte20\n\t" // 1-2  if(bit == 0)
      "rol  %2\n\t"         // 1    b <<= 1       (T = 10)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 12)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 14)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 16)
      "st   %a0, %5\n\t"    // 2    PORT = lo     (T = 18)
      "rjmp head20\n\t"     // 2    -> head20 (next bit out)
     "nextbyte20:\n\t"      //                    (T = 10)
      "nop\n\t"             // 1    nop           (T = 11)
      "ldi  %3, 8\n\t"      // 1    bit = 8       (T = 12)
      "ld   %2, %a6+\n\t"   // 2    b = *ptr++    (T = 14)
      "sbiw %7, 1\n\t"      // 2    i--           (T = 16)
      "st   %a0, %5\n\t"    // 2    PORT = lo     (T = 18)
      "brne head20\n\t"     // 2    if(i != 0) -> head20 (next byte)
      ::
      "e" (port),          // %a0
      "r" (hi),            // %1
      "r" (b),             // %2
      "r" (bit),           // %3
      "r" (next),          // %4
      "r" (lo),            // %5
      "e" (ptr),           // %a6
      "w" (i)              // %7
    ); // end asm

  sei();              // Re-enable interrupts
  endTime = micros(); // Note EOD time for latch on next call
}

// Set pixel color from separate R,G,B components:
void Adafruit_NeoPixel::setPixelColor(
 uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  if(n < numLEDs) {
    uint8_t *p = &pixels[n * 3];
    if((type & NEO_COLMASK) == NEO_GRB) { *p++ = g; *p++ = r; }
    else                                { *p++ = r; *p++ = g; }
    *p = b;
  }
}

// Set pixel color from 'packed' 32-bit RGB color:
void Adafruit_NeoPixel::setPixelColor(uint16_t n, uint32_t c) {
  if(n < numLEDs) {
    uint8_t *p = &pixels[n * 3];
    if((type & NEO_COLMASK) == NEO_GRB) { *p++ = c >>  8; *p++ = c >> 16; }
    else                                { *p++ = c >> 16; *p++ = c >>  8; }
    *p = c;
  }
}

// Convert separate R,G,B into packed 32-bit RGB color.
// Packed format is always RGB, regardless of LED strand color order.
uint32_t Adafruit_NeoPixel::Color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t Adafruit_NeoPixel::getPixelColor(uint16_t n) {

  if(n < numLEDs) {
    uint16_t ofs = n * 3;
    return (uint32_t)(pixels[ofs + 2]) |
      (((type & NEO_COLMASK) == NEO_GRB) ?
        ((uint32_t)(pixels[ofs    ]) <<  8) |
        ((uint32_t)(pixels[ofs + 1]) << 16)
      :
        ((uint32_t)(pixels[ofs    ]) << 16) |
        ((uint32_t)(pixels[ofs + 1]) <<  8) );
  }

  return 0; // Pixel # is out of bounds
}

uint16_t Adafruit_NeoPixel::numPixels(void) {
  return numLEDs;
}
