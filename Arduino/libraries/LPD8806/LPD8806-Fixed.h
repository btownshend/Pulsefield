// From http://michaelnoland.com/speeding-up-lpd8806-show-without-hardware-spi/

static const byte LED_DATA_MASK = 1 << 3 | 1<<4 | 1 << 5 | 1<<6;
static const byte LED_CLOCK_MASK = 1 << 2;

void PulseClockLine(volatile uint8_t& ClockRegister)
{
  ClockRegister |= LED_CLOCK_MASK;
  ClockRegister &= ~LED_CLOCK_MASK;
}
 
void TransmitBit(byte& CurrentByte, volatile uint8_t& ClockRegister, volatile uint8_t& DataRegister)
{
  // Set the data bit
  if (CurrentByte & 0x80)
  {
    DataRegister |= LED_DATA_MASK;
  }
  else
  {
    DataRegister &= ~LED_DATA_MASK;
  }
 
  // Pulse the clock line
  PulseClockLine(ClockRegister);
 
  // Advance to the next bit to transmit
  CurrentByte = CurrentByte << 1;
}
 
  // Specify Arduino pin numbers
void showFixed()
{
    uint8_t ClockRegister=digitalPinToPort(2);
    uint8_t DataRegister=digitalPinToPort(3);
    Serial.println(ClockRegister,HEX);
    Serial.println(DataRegister,HEX);
  // Clock out the color for each LED
  byte* DataPtr = pixels;
  byte* EndDataPtr = pixels + (numLEDs * 3);
 
  do
  {
    byte CurrentByte = *DataPtr++;
 
    TransmitBit(CurrentByte, ClockRegister, DataRegister);
    TransmitBit(CurrentByte, ClockRegister, DataRegister);
    TransmitBit(CurrentByte, ClockRegister, DataRegister);
    TransmitBit(CurrentByte, ClockRegister, DataRegister);
 
    TransmitBit(CurrentByte, ClockRegister, DataRegister);
    TransmitBit(CurrentByte, ClockRegister, DataRegister);
    TransmitBit(CurrentByte, ClockRegister, DataRegister);
    TransmitBit(CurrentByte, ClockRegister, DataRegister);
  }
  while (DataPtr != EndDataPtr);
 
  // Clear the data line while we clock out the latching pattern
  DataRegister &= ~LED_DATA_MASK;
 
  // All of the original data had the high bit set in each byte.  To latch
  // the color in, we need to clock out another LED worth of 0's for every
  // 64 LEDs in the strip apparently.
  byte RemainingLatchBytes = ((numLEDs + 63) / 64) * 3;
  do 
  {
    PulseClockLine(ClockRegister);
    PulseClockLine(ClockRegister);
    PulseClockLine(ClockRegister);
    PulseClockLine(ClockRegister);
 
    PulseClockLine(ClockRegister);
    PulseClockLine(ClockRegister);
    PulseClockLine(ClockRegister);
    PulseClockLine(ClockRegister);
  } while (--RemainingLatchBytes);
 
  // Need a bit of a delay before clocking again, but ideally this
  // is set to 0 and meaningful work is done instead
  if (pause)
  {
    delay(pause);
  }
}
