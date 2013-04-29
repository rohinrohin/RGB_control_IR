#include <EEPROM.h>
#include <IRremote.h>
//#include <IRremoteInt.h>
//#include <SerialCommand.h>

/*
LED Controller for Arduino
By Rohin Gopalakrishnanx
Available on Public Domain.
http://rohinrohin.github.io/RGB_control_IR
Credits to DarkFox and Shirrif

created on Apr29th 2013
*/


// set the ledPins
int redPin = 6;
int greenPin = 5;
int bluePin = 9;

int lastCode;
int code;
int RECV_PIN = 10;
IRrecv irrecv(RECV_PIN);
decode_results results;

//SerialCommand sCmd;

// LED Power variables
byte redPwr = 0;
byte greenPwr = 0;
byte bluePwr = 0;

// Variables for colors to fade to
// set the initial random colors
byte redNew = random(255);
byte greenNew = random(255);
byte blueNew = random(255);

// light mode variable
// initial value 0 = off
int lightMode = 0; // Saved in EEPROM address 0

// Variables for saving the last state before standby
int lastLightMode = 1; // EEPROM address 7

// Variables for colors to fade to
// set the initial random colors
int hueNew = random(360);
int satNew = random(255);
int lumNew = random(255);

int hueVal;
int satVal;
int lumVal;

// misc interface variables
byte menu = 0;

long previousDelayMillis = 0; // Will store last time button was pressed
long buttonDelay = 200;        // Delay before repeating button press

long previousMillis = 0;      // will store last time thing was updated
long interval = 10;           // interval at which to blink (milliseconds) (0 - 4095) Saved in EEPROM address 5+6

int pulseState = 255; // Save where the pulse is. Starting high.
int pulseDir = 0;     // Pulse direction

void setup()
{
//  bitSet(TCCR1B, WGM12);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  loadConfig();

  //pinMode(potPin, INPUT);

  irrecv.enableIRIn(); // Start the receiver

  lightMode = 1;

  // serial
  Serial.begin(9600);

//  // Setup callbacks for SerialCommand commands
//  sCmd.addCommand("MODE",  s_mode); // (int mode) Set light mode
//  sCmd.addCommand("HUE",  s_hue); // (bigint hue) Set hue
//  sCmd.addCommand("SAT",  s_sat); // (int sat) Set saturation
//  sCmd.addCommand("LUM",  s_lum); // (int lum) Set luminescence
//  sCmd.addCommand("INTERVAL",  s_interval); // (bigint interval) Set interval
//  sCmd.addCommand("SETHSL", s_set_hsl); // (bigint hue, int sat, int lum) Set hue, saturation and luminescence in one command
//  sCmd.addCommand("WRITEHSL", s_write_hsl); // Write to LEDs using HSL values
//  sCmd.addCommand("RED", s_red); // (int pwr) Write red power
//  sCmd.addCommand("GRN", s_grn); // (int pwr) Write green power
//  sCmd.addCommand("BLU", s_blu); // (int pwr) Write blue power
//  sCmd.addCommand("SETRGB", s_set_rgb); // (int red, int grn, int blu) Set red, green and blue in one command
//  sCmd.addCommand("WRITERGB", s_write_rgb); // Write to LEDs using RGB values
//  sCmd.addCommand("SAVE", s_save_state); // Save state to EEPROM
//  sCmd.addCommand("STATE", printInfoToSerial); // Get current state
//  sCmd.setDefaultHandler(unrecognized);      // Handler for command that isn't matched

  Serial.println("--RGB LED controller booted and ready--");
}

//// This gets set as the default handler, and gets called when no other command matches.
//void unrecognized(const char *command) {
//  Serial.println("UNRECOGNIZED");
//}
//
//void s_mode() {
//  int aNumber;
//  aNumber = get_serial_number();
//  changeMode(aNumber);
//}
//
//void s_hue() {
//  hueVal = get_serial_number();
//}
//
//void s_sat() {
//  satVal = get_serial_number();
//}
//
//void s_lum() {
//  lumVal = get_serial_number();
//}
//
//void s_interval() {
//  interval = get_serial_number();
//}
//
//void s_set_hsl() {
//  hueVal = get_serial_number();
//  satVal = get_serial_number();
//  lumVal = get_serial_number();
//}
//
//void s_write_hsl() {
//  hslWrite();
//}
//
//void s_red() {
//  redPwr = get_serial_number();
//}
//
//void s_grn() {
//  greenPwr = get_serial_number();
//}
//
//void s_blu() {
//  bluePwr = get_serial_number();
//}
//
//void s_set_rgb() {
//  redPwr = get_serial_number();
//  greenPwr = get_serial_number();
//  bluePwr = get_serial_number();
//  writeLED();
//}
//
//void s_write_rgb() {
//  writeLED();
//}
//
//void s_save_state() {
//  saveState();
//}
//
//int get_serial_number() {
//  int aNumber;
//  char *arg;
//  arg = sCmd.next();
//
//  if (arg != NULL) {
//    aNumber = atoi(arg);
//    return aNumber;
//  } else {
//    Serial.println("ERROR");
//    return NULL;
//  }
//}

void loadConfig() { // Load Previous Config. of EEPROM
  Serial.println("Reading EEPROM for last LED State...");
  lightMode = EEPROM.read(0);

  hueVal = loadBig(1,2);
  satVal = EEPROM.read(3);
  lumVal = EEPROM.read(4);

  interval = loadBig(5,6);

  lastLightMode = EEPROM.read(7);
}

void loop()
{
//  sCmd.readSerial();

  // Save clockcycles when running in serial mode.
//  if (lightMode != 99) {
    getIrCmd();

    switch(lightMode) {
      case 0:
        mdOff();
      break;
      case 1:
        mdSolid();
      break;
      case 2:
        mdPulse();
      break;
      case 3:
        mdRandomFade();
      break;
      case 4:
        mdRainbow();
      break;
      case 5:
        mdStrobe();
      break;
      case 6:
        mdSleep();
      break;
      case 7:
        mdDirect();
      break;
      case 99:
        // Freewheeling, let serial run the show. (Doesn't actually get into this loop)
      break;
      default:
      mdOff();
    }
  }


// Off
void mdOff() {
  lumVal = 0;
  fadeTo(1);
}

// On - Solid
void mdSolid() {
  fadeTo(1);
}

// Pulsing
void mdPulse() {
  if (millis() - previousMillis > interval) {
    previousMillis = millis();

    if (pulseState > lumVal) {
      pulseState = lumVal;
    }

    if (pulseDir == 0) {
      pulseState--;

      if (pulseState == 0) {
        pulseDir = 1;
      }
    } else {
      pulseState++;

      if (pulseState >= 127) {
        pulseDir = 0;
      }
    }

    lumVal = pulseState;

    hslWrite();
  }
}

// Fade to random
void mdRandomFade() {
  if (millis() - previousMillis > interval) {
    previousMillis = millis();

    if (hueVal > hueNew) {
      hueVal--;
    }
    if (hueVal < hueNew) {
      hueVal++;
    }
    if (satVal > satNew) {
      satVal--;
    }
    if (satVal < satNew) {
      satVal++;
    }

    // If all Pwr match New get new colors
    if ((hueVal == hueNew) && (satVal == satNew)) {
      hueNew = random(254);
      satNew = random(254);
    }

    hslWrite();
  }
}

// Rainbow flag fade
void mdRainbow() {
  switch(hueVal) {
    case 0:
    hueVal = 30;
    case 30:
    hueVal = 60;
    case 60:
    hueVal = 120;
    case 120:
    hueVal = 240;
    case 240:
    hueVal = 300;
    case 300:
    hueVal = 0;
  }

  fadeTo(interval);
}

// Strobe
void mdStrobe() {
  if (millis() - previousMillis > interval) {
    previousMillis = millis();

    if (lumVal == 0) {
      lumVal = 127;
    } else {
      lumVal = 0;
    }

    hslWrite();
  }
}

// Sleep
void mdSleep() {
  if (millis() - previousMillis > 8000) {
    previousMillis = millis();

    lumVal--;

    hslWrite();

    if (lumVal == 0) {
      changeMode(0);
      lumVal = 127;
    }
  }
}

// Go directly to the selected color
void mdDirect() {
  Serial.println("Direct Color Function Called. ");
  hslWrite();
}

void menuChoice(int number) {
  Serial.println("menuChoice func. Called. ");
  switch(number) {
    case 1:
      if (menu == 1) {
        changeMode(1);
      } else {
        Serial.println("Red Solid MenuChoice Func. Called");
        turnOn(1);
        hueVal = 0; // Red
      }
      break;

    case 2:
      if (menu == 1) {
        changeMode(2);
      } else {
        turnOn(1);
        Serial.println("Green Solid MenuChoice Func. Called");
        hueVal = 120; // Green
      }
      break;

    case 3:
      if (menu == 1) {
        changeMode(3);
      } else {
        turnOn(1);
        Serial.println("Blue Solid MenuChoice Func. Called");
        hueVal = 240; // Blue
      }
      break;

    case 4:
      if (menu == 1) {
        changeMode(4);
      } else {
        turnOn(1);
        Serial.println("Orange Solid MenuChoice Func. Called");
        hueVal = 10; // Orange
      }
      break;

    case 5:
      if (menu == 1) {
        changeMode(5);
      } else {
        turnOn(1);
        Serial.println("Sea Foam Solid MenuChoice Func. Called");
        hueVal = 135; // Sea Foam
      }
      break;

    case 6:
      if (menu == 1) {
        changeMode(6);
      } else {
        turnOn(1);
        Serial.println("Purple Solid MenuChoice Func. Called");
        hueVal = 270; // Purple
      }
      break;

    case 7:
      if (menu == 1) {
        changeMode(7);
      } else {
        turnOn(1);
        Serial.println("Yellow Solid MenuChoice Func. Called");
        hueVal = 45; // Yellow
      }
      break;

    case 8:
      if (menu == 1) {

      } else {
       turnOn(1);
       Serial.println("Cyan Solid MenuChoice Func. Called");
        hueVal = 180; // Cyan
      }
      break;

    case 9:
      if (menu == 1) {

      } else {
        turnOn(1);
        Serial.println("Pink Solid MenuChoice Func. Called");
        hueVal = 350; // Pink
      }
      break;

    case 0:
      if (menu == 1) {

      } else {
        turnOn(1);
        Serial.println("Light Blue Solid MenuChoice Func. Called");
        hueVal = 210; // Light blue
      }
      break;

    case 10: // Recall
      if (menu == 1) {

      } else {
        turnOn(1);
        hueVal = 80; // Lime
        Serial.println("Lime Solid MenuChoice Func. Called");
      }
      break;
  }
  menu = 0;
}

void getIrCmd() {
  if (irrecv.decode(&results)) {
    //Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
    
Serial.println(results.value, HEX);
    if (results.value != 0xFFFFFFFF) {
      code = results.value;
      lastCode = code;
      Serial.println("IR Code Recieved. ");
      previousDelayMillis = millis();
    } else {
      if (millis() - previousDelayMillis > buttonDelay) {
        code = lastCode;
      } else {
        code = 0x0;
      }
    }

    switch(code) {
      case 163967: // FM
      Serial.println("FM Radio Recieved");
        if (results.value != 0xFFFFFFFF) {
          if (lightMode == 3) {
            changeMode(1);
          }
          hueVal = random(360);
        } else {
          changeMode(3);
        }
        break;

      case 131327: // Power
      
        if (results.value != 0xFFFFFFFF) {
          if (lightMode == 0) {
            changeMode(lastLightMode);
            Serial.println("Power On Recieved ");
          } else {
            changeMode(0);
            Serial.println("Power Off Recieved ");
          }
        }
        break;

      case 180287: // 1{
        {
      Serial.println("IR Decoded - No. 1 Recieved");
        menuChoice(1);
        break;
        }
      case 139487: // 2
      Serial.println("IR Decoded - No. 2 Recieved");
        menuChoice(2);
        break;

      case 172127: // 3 
      Serial.println("IR Decoded - No. 3 Recieved");
        menuChoice(3);
        break;

      case 155807: // 4
      Serial.println("IR Decoded - No. 4 Recieved");
        menuChoice(4);
        break;

      case 188447: // 5
      Serial.println("IR Decoded - No. 5 Recieved");
        menuChoice(5);
        break;

      case 135407: // 6
      Serial.println("IR Decoded - No. 6 Recieved");
        menuChoice(6);
        break;

      case 168047: // 7
      Serial.println("IR Decoded - No. 7 Recieved");
        menuChoice(7);
        break;

      case 151727: // 8
      Serial.println("IR Decoded - No. 8 Recieved");
        menuChoice(8);
        break;

      case 184367: // 9
      Serial.println("IR Decoded - No. 9 Recieved");
        menuChoice(9);
        break;

      case 143567: // 0
      Serial.println("IR Decoded - No. 0 Recieved");
        menuChoice(0);
        break;

      case 157847: // Recall
      Serial.println("IR Decoded - Recall Key Recieved");
        menuChoice(10);
        break;

      case 159887: // Menu
      Serial.println("IR Decoded - Menu Button Recieved");
        if (menu == 0) {
          menu = 1;
        } else {
          menu = 0;
        }
        break;

      case 101010: // +100
        turnOn(1);
        satVal = 0; // White
        break;

      case 152237: // Info = Message Hathway
      Serial.println("Print Info Request");
        if (results.value != 0xFFFFFFFF) {
          printInfoToSerial();
        }
        break;

      case 166007: // Up
      Serial.println("IR Decoded - Remote UP Recieved");
        if (results.value != 0xFFFFFFFF) {
          satVal++;
        } else {
          satVal = satVal+10;
        }
        if (satVal > 255) {
          satVal = 255;
        }
        break;

      case 174167: // Down
      Serial.println("IR Decoded - Remote DOWN Recieved");
        if (results.value != 0xFFFFFFFF) {
          satVal--;
        } else {
          satVal = satVal-10;
        }
        if (satVal < 0) {
          satVal = 0;
        }
        break;

      case 149687: // Left
      Serial.println("IR Decoded - Remote Left Recieved");
        if (results.value != 0xFFFFFFFF) {
          hueVal--;
        } else {
          hueVal = hueVal-10;
        }
        if (hueVal < 0) {
          hueVal = hueVal+360;
        }
        break;

      case 141527: // Right
      Serial.println("IR Decoded - Remote Right Recieved");
        if (results.value != 0xFFFFFFFF) {
          hueVal++;
        } else {
          hueVal = hueVal+10;
        }
        if (hueVal > 360) {
          hueVal = hueVal-360;
        }
        break;

      case 182327: // OK
      Serial.println("IR Decoded - Remote OK Recieved");
        satVal = 255;
        break;

      case 010101: // Mode
        interval = 50;
        break;

      case 194561: // Vol+
      Serial.println("IR Decoded - Remote V+ Recieved");
        if (results.value != 0xFFFFFFFF) {
          lumVal++;
        } else {
          lumVal = lumVal+10;
        }
        if (lumVal > 255) {
          lumVal = 255;
        }
        break;

      case 131837: // Vol-
      Serial.println("IR Decoded - Remote V- Recieved");
        if (results.value != 0xFFFFFFFF) {
          lumVal--;
        } else {
          lumVal = lumVal-10;
        }
          if (lumVal < 0) {
          lumVal = 0;
        }
        break;

      case 137447: // Mute
      Serial.println("IR Decoded - Remote Mute Recieved");
        lumVal = 127;
        break;

      case 133367: // Chan+
      Serial.println("IR Decoded - Remote CH+ Recieved");
        if (results.value != 0xFFFFFFFF) {
          interval++;
        } else {
          interval = interval+5;
        }
        if (interval > 4095) {
          interval = 4095;
        }
        break;

      case 192527: // Chan-
      Serial.println("IR Decoded - Remote CH- Recieved");
        if (results.value != 0xFFFFFFFF) {
          interval--;
        } else {
          interval = interval-5;
        }
        if (interval < 0) {
          interval = 0;
        }
        break;

      default:
        break;
    }

    saveState();
  } // End IR control
}

void saveState() {
  EEPROM.write(0, lightMode);
  saveBig(hueVal, 1, 2);
  EEPROM.write(3, satVal);
  EEPROM.write(4, lumVal);
  saveBig(interval, 5, 6);
}

void printInfoToSerial() {
  Serial.print("{");
  Serial.print("\"mode\":");
  Serial.print(lightMode, DEC);
  Serial.print(",\"hue\":");
  Serial.print(hueVal, DEC);
  Serial.print(",\"sat\":");
  Serial.print(satVal, DEC);
  Serial.print(",\"lum\":");
  Serial.print(lumVal, DEC);
  Serial.print(",\"interval\":");
  Serial.print(interval, DEC);
  Serial.print(",\"red\":");
  Serial.print(redPwr, DEC);
  Serial.print(",\"grn\":");
  Serial.print(greenPwr, DEC);
  Serial.print(",\"blu\":");
  Serial.print(bluePwr, DEC);
  Serial.println("}");
}

void changeMode(int mode) {
  Serial.println("Mode Func. ");
  switch(mode) {
    case 0:
      lastLightMode = lightMode;
      EEPROM.write(7, lastLightMode);
      break;
    case 1:
      lumVal = 127;
      break;
    case 3:
      lumVal = 127;
      break;
    case 4:
      hueVal = 0;
      break;
    case 5:
      lumVal = 127;
      break;
  }

  lightMode = mode;
  menu = 0;
}

void turnOn(int mode) {
  if (lightMode == 0) {
    changeMode(mode);
  }
}

void fadeTo(int fadeSpeed) {
  byte rgb[3];
  hslToRgb( hueVal, satVal, lumVal, rgb );

  redNew = rgb[0];
  greenNew = rgb[1];
  blueNew = rgb[2];

  if (millis() - previousMillis > fadeSpeed) {
    // save the last time you blinked the LED
    previousMillis = millis();
    if (redPwr > redNew) {
      redPwr--;
    }
    if (redPwr < redNew) {
      redPwr++;
    }
    if (greenPwr > greenNew) {
      greenPwr--;
    }
    if (greenPwr < greenNew) {
      greenPwr++;
    }
    if (bluePwr > blueNew) {
      bluePwr--;
    }
    if (bluePwr < blueNew) {
      bluePwr++;
    }
    writeLED();
  }
}


//-------------------------------------------------------------------
// -- hsl output --
//

void hslWrite()
{
    byte rgb[3];
    hslToRgb( hueVal, satVal, lumVal, rgb );

    redPwr = rgb[0];
    greenPwr = rgb[1];
    bluePwr =  rgb[2];
    writeLED();
}

// thx to -- http://www.kasperkamperman.com/blog/arduino/arduino-programming-hsb-to-rgb/
// edit by Rohin Gopalakrishnan
const byte gamma_curve[] = {
    0,   1,   1,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
    3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,   4,   4,   4,
    4,   4,   4,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   6,   6,   6,
    6,   6,   6,   6,   6,   7,   7,   7,   7,   7,   7,   7,   8,   8,   8,   8,
    8,   8,   9,   9,   9,   9,   9,   9,   10,  10,  10,  10,  10,  11,  11,  11,
    11,  11,  12,  12,  12,  12,  12,  13,  13,  13,  13,  14,  14,  14,  14,  15,
    15,  15,  16,  16,  16,  16,  17,  17,  17,  18,  18,  18,  19,  19,  19,  20,
    20,  20,  21,  21,  22,  22,  22,  23,  23,  24,  24,  25,  25,  25,  26,  26,
    27,  27,  28,  28,  29,  29,  30,  30,  31,  32,  32,  33,  33,  34,  35,  35,
    36,  36,  37,  38,  38,  39,  40,  40,  41,  42,  43,  43,  44,  45,  46,  47,
    48,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,
    63,  64,  65,  66,  68,  69,  70,  71,  73,  74,  75,  76,  78,  79,  81,  82,
    83,  85,  86,  88,  90,  91,  93,  94,  96,  98,  99,  101, 103, 105, 107, 109,
    110, 112, 114, 116, 118, 121, 123, 125, 127, 129, 132, 134, 136, 139, 141, 144,
    146, 149, 151, 154, 157, 159, 162, 165, 168, 171, 174, 177, 180, 183, 186, 190,
    193, 196, 200, 203, 207, 211, 214, 218, 222, 226, 230, 234, 238, 242, 248, 255,
};


// thanks to -- http://www.dipzo.com/wordpress/?p=50
// re-edit - Rohin Gopi
void hslToRgb( int hue, byte sat, byte lum, byte* lpOutRgb )
{
  if( sat == 0 )
  {
    lpOutRgb[0] = lpOutRgb[1] = lpOutRgb[2] = lum;
    return;
  }

  float nhue   = (float)hue * (1.0f / 360.0f);
  float nsat   = (float)sat * (1.0f / 255.0f);
  float nlum   = (float)lum * (1.0f / 255.0f);

  float m2;
  if( lum < 128 )
    m2 = nlum * ( 1.0f + nsat );
  else
    m2 = ( nlum + nsat ) - ( nsat * nlum );

  float m1 = ( 2.0f * nlum ) - m2;

  lpOutRgb[0] = hueToChannel( m1, m2, nhue + (1.0f / 3.0f) );
  lpOutRgb[1] = hueToChannel( m1, m2, nhue );
  lpOutRgb[2] = hueToChannel( m1, m2, nhue - (1.0f / 3.0f) );
}

inline byte hueToChannel( float m1, float m2, float h )
{
  return hueToChannelRaw( m1, m2, h );
}

byte hueToChannelRaw( float m1, float m2, float h )
{
  float channel = hueToChannelInternal( m1, m2, h );
  byte  uchan   = (byte)(255.0f * channel);
  return uchan;
}

// Thanks - DarthVader(@DArthy)
byte hueToChannelGamma( float m1, float m2, float h )
{
  float channel = hueToChannelInternal( m1, m2, h );
  byte  uchan   = (byte)(255.0f * channel);
  return gamma_curve[uchan];
}

float hueToChannelInternal( float m1, float m2, float h )
{
   if( h < 0.0f ) h += 1.0f;
   if( h > 1.0f ) h -= 1.0f;

   if( ( 6.0f * h ) < 1.0f )  return ( m1 + ( m2 - m1 ) * 6.0f * h );
   if( ( 2.0f * h ) < 1.0f )  return m2;
   if( ( 3.0f * h ) < 2.0f )  return ( m1 + ( m2 - m1 ) * ((2.0f/3.0f) - h) * 6.0f );
   return m1;
}

void writeLED() {
  // display the colors
  analogWrite(redPin, redPwr);
  analogWrite(greenPin, greenPwr);
  analogWrite(bluePin, bluePwr);
}

void saveBig(int value, int address0, int address1) {
  int value1 = value / 255;
  int value2 = value % 255;

  EEPROM.write(address0, value1);
  EEPROM.write(address1, value2);
}

int loadBig(int address0, int address1) {
  int v1 = EEPROM.read(address0);
  int v2 = EEPROM.read(address1);
  return v1*255+v2;
}
