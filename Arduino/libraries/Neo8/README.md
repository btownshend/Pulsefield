WS2811 Driver
-------------
Modified from Adafruit NeoPixel to handle 8 LED strips in parallel connected to individual bits of a single register
Assumes data is loaded into memory in correct format to shift out in parallel as-is

Based on:
Adafruit NeoPixel library
=========================

Arduino library for controlling single-wire-based LED pixels and strip such as the [Adafruit 60 LED/meter Digital LED strip][strip] and the [Adafruit FLORA RGB Smart Pixel][pixel].

After downloading, rename folder to 'Adafruit_NeoPixel' and install in Arduino Libraries folder. Restart Arduino IDE, then open File->Sketchbook->Library->Adafruit_NeoPixel->strandtest sketch.

[pixel]: http://adafruit.com/products/1060
[strip]:  http://adafruit.com/products/1138