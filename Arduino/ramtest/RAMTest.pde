/* This MegaRAM application is a production test that verifies that
 * all RAM locations are unique and can store values.
 *
 * Assumptions:
 *
 *   - MegaRAM plugged in to Arduino Mega 1280 or Mega 2560
 *
 * The behavior is as follows:
 *
 *   - Each one of 4 banks of 32 kbytes each is filled with a pseudorandom sequence of numbers
 *   - The 4 banks are read back to verify the pseudorandom sequence
 *   - On success, the Arduino's LED is turned on steadily, with a brief pulse off every 3 seconds
 *   - On failure, the Arduino's LED blinks rapidly
 *   - The serial port is used for status information (38400 bps)
 *
 * This software is licensed under the GNU General Public License (GPL) Version
 * 3 or later. This license is described at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Application Version 1.0 -- August 2011 Rugged Circuits LLC
 * http://www.ruggedcircuits.com/html/megaram.html
 */

void setup(void)
{
  DDRD |= _BV(7); // PD7 is BANKSEL
  PORTD = 0x7F;   // Select bank 0, pullups on everything else

  /* Enable XMEM interface:

         SRE    (7)   : Set to 1 to enable XMEM interface
         SRL2-0 (6-4) : Set to 00x for wait state sector config: Low=N/A, High=0x2200-0xFFFF
         SRW11:0 (3-2) : Set to 00 for no wait states in upper sector
         SRW01:0 (1-0) : Set to 00 for no wait states in lower sector
   */
  XMCRA = 0x80;

  // Bus keeper, lower 7 bits of Port C used for upper address bits, bit 7 is under PIO control
  XMCRB = _BV(XMBK) | _BV(XMM0);
  DDRC |= _BV(7);
  PORTC = 0x7F; // Enable pullups on port C, select bank 0

  // PL7 is RAM chip enable, active high. Enable it now, and enable pullups on Port L.
  DDRL |= _BV(7);
  PORTL = 0xFF;

  // Open up a serial channel
  Serial.begin(38400);

  // Enable on-board LED
  pinMode(13, OUTPUT);

  Serial.println("Memory test begins ...");
}

// Blink the Arduino LED quickly to indicate a memory test failure
void blinkfast(void)
{
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  delay(100);
}

// Provide information on which address failed. Then, sit in an infinite loop blinking
// the LED quickly.
void fail(uint8_t *addr, uint8_t expect, uint8_t got)
{
  Serial.print("At address "); Serial.print((uint16_t)addr);
  Serial.print(" expected "); Serial.print(expect, DEC);
  Serial.print(" got "); Serial.print(got, DEC);
  Serial.println();

  while (1) {
    blinkfast();
  }
}

#define STARTADDR ((uint8_t *)0x8000)
#define COUNT (0x8000-1)

long seed=1234;

// Select one of four 32 kilobyte banks
void banksel(uint8_t bank)
{
  switch (bank) {
    case 0:
      PORTD &= ~_BV(7);
      PORTC &= ~_BV(7);
      break;

    case 1:
      PORTD &= ~_BV(7);
      PORTC |=  _BV(7);
      break;

    case 2:
      PORTD |=  _BV(7);
      PORTC &= ~_BV(7);
      break;

    default:
      PORTD |=  _BV(7);
      PORTC |=  _BV(7);
      break;
  }
}

// Fill the currently selected 32 kilobyte bank of memory with a random sequence
void bankfill(void)
{
  uint8_t *addr;
  uint16_t count;

  for (addr=STARTADDR, count=COUNT; count; addr++, count--) {
    *addr = (uint8_t)random(256);
    if ((count & 0xFFFU) == 0) blinkfast();
  }

}

// Check the currently selected 32 kilobyte bank of memory against expected values
// in each memory cell.
void bankcheck(void)
{
  uint8_t expect;
  uint8_t *addr;
  uint16_t count;

  for (addr=STARTADDR, count=COUNT; count; addr++, count--) {
    expect = random(256);
    if (*addr != expect) fail(addr,expect,*addr);
    if ((count & 0xFFFU) == 0) blinkfast();
  }
}

void loop(void)
{
  uint8_t bank;

  // Always start filling and checking with the same random seed
  randomSeed(seed);

  // Fill all 4 banks with random numbers
  for (bank=0; bank < 4; bank++) {
    Serial.print("   Filling bank "); Serial.print(bank, DEC); Serial.println(" ...");
    banksel(bank); bankfill();
  }

  // Restore the initial random seed, then check all 4 banks against expected random numbers
  randomSeed(seed);
  for (bank=0; bank < 4; bank++) {
    Serial.print("  Checking bank "); Serial.print(bank, DEC); Serial.println(" ...");
    banksel(bank); bankcheck();
  }

  // If we got this far, there were no failures
  Serial.println("Success!");
  Serial.println();

  // Keep the LED on mostly, and pulse off briefly every 3 seconds
  while (1) {
    digitalWrite(13, HIGH);
    delay(3000);
    digitalWrite(13, LOW);
    delay(500);
  }
}
// vim: syntax=cpp ai ts=2 sw=2 cindent expandtab
