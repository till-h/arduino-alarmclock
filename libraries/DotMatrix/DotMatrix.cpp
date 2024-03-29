/*
  DotMatrix.cpp

  Library for controlling the 3 by 8x8 dot matrix display.

  Created by Till Hackler, December 2015

*/

#include <DotMatrix.h>

DotMatrix::DotMatrix() {}
void DotMatrix::setup(uint8_t dta, uint8_t clk, uint8_t cs,
                      uint8_t brightness, uint32_t interval)
{
    lc.setup(dta, clk, cs, 3); // 3 8x8 segments
    for(uint8_t i = 0; i < lc.getDeviceCount(); i++)
    {
        lc.clearDisplay(i);
        lc.shutdown(i, false);
        lc.setIntensity(i, brightness);
    }
    for (uint8_t i = 0; i < 24; i++)
    {
        display_cache[i] = 0U;
        display[i] = 0U;
    }
    
    _interval = interval;
    _showing = false;
}

void DotMatrix::displayTime(aTime time)
{
    // extract digits
    uint8_t h0 = time.h / 10; // tens of hours
    uint8_t h1 = time.h % 10; // hours
    uint8_t m0 = time.m / 10; // tens of minutes
    uint8_t m1 = time.m % 10; // minutes

    clearCache();

    if (h0 != 0) // skip leading zero
    {
        for (uint8_t col = 0; col < 5; col++)
            setCacheColumn(col, n[h0][col]);
    }
    for (uint8_t col = 6; col < 11; col++)
        setCacheColumn(col, n[h1][col - 6]);
    for (uint8_t col = 13; col < 18; col++)
        setCacheColumn(col, n[m0][col - 13]);
    for (uint8_t col = 19; col < 24; col++)
        setCacheColumn(col, n[m1][col - 19]);

    updateDisplayFromCache();

}

void DotMatrix::blinkTime(aTime time)
{
    uint32_t now = micros();

    if (now - _lastFlank > _interval)
    {
        if (_showing == false)
        {
            displayTime(time);
            _showing = true;
        }
        else
        {
            clearDisplay();
            _showing = false;
        }
        _lastFlank = now;
    }
}

void DotMatrix::displayAlarm(bool status)
{
        clearCache();

        uint8_t col = 0;
        // display clock symbol
        for (; col < 9; col++)
            setCacheColumn(col, clock[col]);
        // display on / off
        for (; col < 24; col++)
            setCacheColumn(col, onoff[int8_t(status)][col - 9]);

        updateDisplayFromCache();
}

void DotMatrix::setCacheColumn(uint8_t col, uint8_t value)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        uint8_t * row = &display_cache[(col / 8) * 8 + i];
        copyBit(value, i, row, col % 8);
    }
}

void DotMatrix::copyBit(const uint8_t source, uint8_t source_index,  uint8_t * const target, const uint8_t target_index)
{
    // 7 - ... terms so that index = 0 refers to MSB
    if ((source & (1U << 7 - source_index)) == 0U)
    {
        *target = *target & ~(1U << 7 - target_index);
    }
    else
    {
        *target = *target | (1U << 7 - target_index);
    }
}

void DotMatrix::updateDisplayFromCache()
{
    for (uint8_t i = 0; i < 24; i++)
    {
        if (display_cache[i] != display[i])
        {
            lc.setRow(i / 8, i % 8, display_cache[i]);
            display[i] = display_cache[i];
        }
    }
}

void DotMatrix::clearCache()
{
    for (uint8_t i = 0; i < 24; i++)
    {
        display_cache[i] = 0U;
    } 
}

void DotMatrix::clearDisplay()
{
    for (uint8_t i = 0; i < lc.getDeviceCount(); i++)
    {
        lc.clearDisplay(i);
    }
    for (uint8_t i = 0; i < 24; i++)
    {
        display_cache[i] = 0;
        display[i] = 0U;
    }  
}

#ifdef CALLIGRAPHY
byte const DotMatrix::n[10][5] =
{
    { // 0
        B00011111,
        B00100001,
        B01000010,
        B10000100,
        B11111000
    },
    { // 1
        B00100000,
        B00100011,
        B01001100,
        B01110000,
        B11000000
    },
    { // 2
        B00100001,
        B01000111,
        B11001010,
        B01110010,
        B00000100
    },
    { // 3
        B00100000,
        B01000000,
        B10001001,
        B10110010,
        B11001100
    },
    { // 4
        B00011000,
        B01101000,
        B10010011,
        B00111100,
        B00010000
    },
    { // 5
        B00110010,
        B01001001,
        B01010001,
        B10010001,
        B00001110
    },
    { // 6
        B00001100,
        B00110010,
        B01001001,
        B10001010,
        B00000100
    },
    { // 7
        B01000011,
        B10001100,
        B10010000,
        B10100000,
        B11000000
    },
    { // 8
        B00000111,
        B01101001,
        B10010001,
        B10101110,
        B11000000
    },
    { // 9
        B00000001,
        B01100001,
        B10010110,
        B10011000,
        B01100000
    }
};
#endif
#ifdef SANS
byte const DotMatrix::n[10][5] =
{
    { // 0
        B00000000,
        B11111111,
        B10000001,
        B11111111,
        B00000000
    },
    { // 1
        B00000000,
        B00000000,
        B11111111,
        B00000000,
        B00000000
    },
    { // 2
        B00000000,
        B10011111,
        B10010001,
        B11110001,
        B00000000
    },
    { // 3
        B00000000,
        B10010001,
        B10010001,
        B11111111,
        B00000000
    },
    { // 4
        B00000000,
        B11110000,
        B00010000,
        B11111111,
        B00000000
    },
    { // 5
        B00000000,
        B11110001,
        B10010001,
        B10011111,
        B00000000
    },
    { // 6
        B00000000,
        B11111111,
        B10010001,
        B10011111,
        B00000000
    },
    { // 7
        B00000000,
        B10000000,
        B10000000,
        B11111111,
        B00000000
    },
    { // 8
        B00000000,
        B11111111,
        B10010001,
        B11111111,
        B00000000
    },
    { // 9
        B00000000,
        B11110000,
        B10010000,
        B11111111,
        B00000000
    }
};
#endif

byte const DotMatrix::clock[9] =
{
    B01000000,
    B10111001,
    B01010101,
    B10010010,
    B10110010,
    B10000010,
    B01000101,
    B10111001,
    B01000000
};

byte const DotMatrix::onoff[2][15] =
{
    { // OFF
        B00000000,
        B00011100,
        B00100010,
        B01000010,
        B01000100,
        B00111000,
        B00000000,
        B00000010,
        B00111100,
        B01010000,
        B01000000,
        B00000010,
        B00111100,
        B01010000,
        B01000000
    },
    { // ON
        B00000000,
        B00011100,
        B00100010,
        B01000010,
        B01000100,
        B00111000,
        B00000000,
        B00000000,
        B00011110,
        B01100000,
        B00011000,
        B00000110,
        B01111000,
        B00000000,
        B00000000
    }
};
