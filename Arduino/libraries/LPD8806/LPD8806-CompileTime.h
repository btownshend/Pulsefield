// From http://michaelnoland.com/speeding-up-lpd8806-show-without-hardware-spi/

template<unsigned int ClockPin>
void PulseClockLine(volatile uint8_t& ClockRegister)
{
  const byte LED_CLOCK_MASK = 1 << ClockPin;
  ClockRegister |= LED_CLOCK_MASK;
  ClockRegister &= ~LED_CLOCK_MASK;
}
 
template<unsigned int ClockPin, unsigned int DataPin>
void TransmitBit(byte& CurrentByte, volatile uint8_t& ClockRegister, volatile uint8_t& DataRegister)
{
  // Set the data bit
  const byte LED_DATA_MASK = 1 << DataPin;
  if (CurrentByte & 0x80)
  {
    DataRegister |= LED_DATA_MASK;
  }
  else
  {
    DataRegister &= ~LED_DATA_MASK;
  }
 
  // Pulse the clock line
  PulseClockLine<ClockPin>(ClockRegister);
 
  // Advance to the next bit to transmit
  CurrentByte = CurrentByte << 1;
}
 
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
  #define MAP_ARDUINO_PIN_TO_PORT_PIN(ArduinoPin) \
    ( ArduinoPin & 7 )
 
  #define MAP_ARDUINO_PIN_TO_PORT_REG(ArduinoPin) \
    ( (ArduinoPin >= 16) ? PORTC : (((ArduinoPin) >= 8) ? PORTB : PORTD) )
 
  // Specify Arduino pin numbers
  template<unsigned int ClockPin, unsigned int DataPin>
  void showCompileTime()
  {
    showCompileTime<MAP_ARDUINO_PIN_TO_PORT_PIN(ClockPin), MAP_ARDUINO_PIN_TO_PORT_PIN(DataPin)>(
      MAP_ARDUINO_PIN_TO_PORT_REG(ClockPin),
      MAP_ARDUINO_PIN_TO_PORT_REG(DataPin));
  }
 
  #undef MAP_ARDUINO_PIN_TO_PORT_PIN
  #undef MAP_ARDUINO_PIN_TO_PORT_REG
#else
  // Sorry: Didn't write an equivalent for other boards; use the other
  // overload and explicitly specify ports and offsets within those ports
#endif
 
// Note: Pin template params need to be relative to their port (0..7), not Arduino pinout numbers
template<unsigned int ClockPin, unsigned int DataPin>
void showCompileTime(volatile uint8_t& ClockRegister, volatile uint8_t& DataRegister)
{
  // Clock out the color for each LED
  byte* DataPtr = pixels;
  byte* EndDataPtr = pixels + (numLEDs * 3);
 
  do
  {
    byte CurrentByte = *DataPtr++;
 
    TransmitBit<ClockPin, DataPin>(CurrentByte, ClockRegister, DataRegister);
    TransmitBit<ClockPin, DataPin>(CurrentByte, ClockRegister, DataRegister);
    TransmitBit<ClockPin, DataPin>(CurrentByte, ClockRegister, DataRegister);
    TransmitBit<ClockPin, DataPin>(CurrentByte, ClockRegister, DataRegister);
 
    TransmitBit<ClockPin, DataPin>(CurrentByte, ClockRegister, DataRegister);
    TransmitBit<ClockPin, DataPin>(CurrentByte, ClockRegister, DataRegister);
    TransmitBit<ClockPin, DataPin>(CurrentByte, ClockRegister, DataRegister);
    TransmitBit<ClockPin, DataPin>(CurrentByte, ClockRegister, DataRegister);
  }
  while (DataPtr != EndDataPtr);
 
  // Clear the data line while we clock out the latching pattern
  const byte LED_DATA_MASK = 1 << DataPin;
  DataRegister &= ~LED_DATA_MASK;
 
  // All of the original data had the high bit set in each byte.  To latch
  // the color in, we need to clock out another LED worth of 0's for every
  // 64 LEDs in the strip apparently.
  byte RemainingLatchBytes = ((numLEDs + 63) / 64) * 3;
  do 
  {
    PulseClockLine<ClockPin>(ClockRegister);
    PulseClockLine<ClockPin>(ClockRegister);
    PulseClockLine<ClockPin>(ClockRegister);
    PulseClockLine<ClockPin>(ClockRegister);
 
    PulseClockLine<ClockPin>(ClockRegister);
    PulseClockLine<ClockPin>(ClockRegister);
    PulseClockLine<ClockPin>(ClockRegister);
    PulseClockLine<ClockPin>(ClockRegister);
  } while (--RemainingLatchBytes);
 
  // Need a bit of a delay before clocking again, but ideally this
  // is set to 0 and meaningful work is done instead
  if (pause)
  {
    delay(pause);
  }
}
