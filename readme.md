### SimpleVario 1.0!!
Pedro Enrique

Components I built:

    - SimpleVario
    - SimpleArray
    - SimpleGPS
    - IGCFileReader
    - Arduino.ino
    - Units
    - Settings
    - Button

Dependencies:

    - SdFat
    - TimeLib
    - MS5611
    - i2c_t3
    - LiquidCrystal_I2C

Once built, turn on the vario once, after it boots, a settings file gets
created, turn it off now and get the SD card out. Modify that `SETTINGS.TXT` 
file. This file will look like this.

```
------------------------------------------
UNIT_SYSTEM=IMPERIAL
CLIMB_THRESHOLD=20.00
SINK_THRESHOLD=-300.00
BEEP_ON_START=TRUE
SINK_ALARM_ON=TRUE
TIMEZONE_UTC=-7
SOUND_OFF=FALSE
------------------------------------------
PILOT_NAME=Pedro Enrique
GLIDER_TYPE=Wills Wing - Sport 2 136
------------------------------------------
```

Wire the components to the Teesy 3.2 board as follows:

```
    Component       Teensy 3.2
        Pin             Pin

    LCD with I2C module:
        VCC . . . . . . VCC
        GND . . . . . . GND
        SLC . . . . . . 19
        SDA . . . . . . 18

    SD Card:
        GND . . . .  . . GND
        C0
        MISO/DO  . . . . 12
        SCK. . . . . . . 13
        MOSI/DI  . . . . 11
        CS . . . . . . . 10
        VCC. . . . . . . VCC

    MS5611
        VCC . . . . . . VCC
        GND . . . . . . GND
        SLC . . . . . . 19
        SDA . . . . . . 18

    GPS:
        BLACK . . . . . GND
        GREEN . . . . .  1
        BLUE  . . . . .  0
        RED . . . . . . VCC
    
    Buzzer or 8W Speaker (use amplifier for better sound):
        POS . . . . . . 21
        NEG . . . . . . GND

    Button Menu
        POS . . . . . . 14
        NEG . . . . . . GND

    Button Up
        POS . . . . . . 16
        NEG . . . . . . GND

    Button Down
        POS . . . . . . 15
        NEG . . . . . . GND
```
