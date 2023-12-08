#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 300

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 52

#define STATE_COLOR     0
#define STATE_WHITE     1
#define STATE_FADE_IN   2
#define STATE_LIT       3
#define STATE_FADE_OUT  4
#define STATE_BLACK     5

#define COLOR_RED     0
#define COLOR_ORANGE  1
#define COLOR_YELLOW  2
#define COLOR_GREEN   3
#define COLOR_BLUE    4
#define COLOR_PURPLE  5
#define COLOR_WHITE   6

// Define the array of leds
CRGB leds[NUM_LEDS];
byte color[NUM_LEDS];
byte colorState;
byte pixelState;

int index;

void setColor()
{
  for(int i = 0; i < NUM_LEDS; i++)
  {
    if (colorState == STATE_COLOR)
    {
      color[i] = random(6);
    }
    else
    {
      color[i] = COLOR_WHITE;
    }
  }
}

void displayColor(byte brightness)
{
  if (brightness >= 0)
  {
    // black cut-off
    if (brightness <= 25)
    {
      brightness = 0;
    }
    for(int i = 0; i < NUM_LEDS; i++)
    {       
      switch (color[i])
      {
        case COLOR_RED:    leds[i] = CHSV(HSVHue::HUE_RED, 255, brightness);     break;
        case COLOR_ORANGE: leds[i] = CHSV(HSVHue::HUE_ORANGE, 255, brightness);  break;
        case COLOR_YELLOW: leds[i] = CHSV(HSVHue::HUE_YELLOW, 255, brightness);  break;
        case COLOR_GREEN:  leds[i] = CHSV(HSVHue::HUE_GREEN, 255, brightness);   break;
        case COLOR_BLUE:   leds[i] = CHSV(HSVHue::HUE_BLUE, 255, brightness);    break;
        case COLOR_PURPLE: leds[i] = CHSV(HSVHue::HUE_PURPLE, 255, brightness);  break;
        case COLOR_WHITE:  leds[i] = CHSV(30, 150, brightness);                  break;
      }
    }
  }
  FastLED.show();
}

void handleState()
{
  switch (pixelState)
  {
    case STATE_FADE_IN:
      {
        displayColor(200 - index);
        index = index - 1;
      }
      break;
    case STATE_FADE_OUT:
      {
        displayColor(index);
        index = index - 1;
      }
      break;
    case STATE_LIT:
    case STATE_BLACK:
      {
        index--;
      }
      break;
  }
  
  if (index < 0)
  {
    nextState();
  }
}

void nextState()
{
  switch (pixelState)
  {
    case STATE_BLACK:
      {
        if (colorState == STATE_COLOR)
        {
          colorState = STATE_WHITE;
        }
        else
        {
          colorState = STATE_COLOR;
        }
        setColor();
        pixelState = STATE_FADE_IN;
        index = 200;
      }
      break;
    case STATE_FADE_IN:
      {
        pixelState = STATE_LIT;
        index = random(500, 1000);
      }
      break;
    case STATE_LIT:
      {
        pixelState = STATE_FADE_OUT;
        index = 200;
      }
      break;
    case STATE_FADE_OUT:
      {
        pixelState = STATE_BLACK;
        index = 50;
      }
      break;
  }
}


void serialInfo()
{
  Serial.print("\nloop: pixelState=");
  Serial.print(pixelState);
  Serial.print(", colorState=");
  Serial.print(colorState);
  Serial.print(", index=");  
  Serial.print(index);
  Serial.print(", color[0]=");
  Serial.print(color[0]);
  Serial.print(", color[1]=");
  Serial.print(color[1]);
  Serial.print(", color[2]=");
  Serial.print(color[2]);
  Serial.print(", color[3]=");
  Serial.print(color[3]);
  Serial.print(", color[4]=");
  Serial.print(color[4]);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nresetting");
  FastLED.addLeds<WS2811,DATA_PIN,RGB>(leds,NUM_LEDS);
  colorState = STATE_COLOR;
  setColor();
  pixelState = STATE_BLACK;
}
 
void loop() { 
  // put your main code here, to run repeatedly:
  serialInfo();
  handleState(); 
  delay(15);
  // static byte hue = 0;
}
