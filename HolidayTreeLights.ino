#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 300

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 52

#define STATE_COLOR      0
#define STATE_RAINBOW    1
#define STATE_WHITE      2

#define STATE_LINKED     0
#define STATE_UNLINKED   1

#define PIXEL_FADE_IN    1
#define PIXEL_LIT        2
#define PIXEL_FADE_OUT   3
#define PIXEL_BLACK      4
#define PIXEL_ENDING     5

#define COLOR_RED     0
#define COLOR_ORANGE  1
#define COLOR_YELLOW  2
#define COLOR_GREEN   3
#define COLOR_BLUE    4
#define COLOR_PURPLE  5

// Define the array of leds
CRGB leds[NUM_LEDS];
byte hue[NUM_LEDS];
byte colorState;
byte pixelState[NUM_LEDS];
byte linkedState;
int index[NUM_LEDS];
int litMin, litMax, darkMin, darkMax, fadeRate;
int endingCounter;

void setHueAll()
{
  for(int i = 0; i < NUM_LEDS; i++)
  {
    setHue(i);
  }
}

void setHue(int pixel)
{
  if (colorState == STATE_COLOR)
  {
    hue[pixel] = random(256);
  }
  else if (colorState == STATE_RAINBOW)
  {
    byte newColor = random(6);
    switch (newColor)
    {
      case COLOR_RED:    hue[pixel] = HSVHue::HUE_RED;     break;
      case COLOR_ORANGE: hue[pixel] = HSVHue::HUE_ORANGE;  break;
      case COLOR_YELLOW: hue[pixel] = HSVHue::HUE_YELLOW;  break;
      case COLOR_GREEN:  hue[pixel] = HSVHue::HUE_GREEN;   break;
      case COLOR_BLUE:   hue[pixel] = HSVHue::HUE_BLUE;    break;
      case COLOR_PURPLE: hue[pixel] = HSVHue::HUE_PURPLE;  break;
    }
  }
  else  // colorState == STATE_WHITE
  {
    // warm white
    hue[pixel] = 30;
  }
}

void displayColor(int pixel, byte brightness)
{
  if (brightness >= 0)
  {
    byte saturation;
  
    if (colorState == STATE_WHITE)
    {
      saturation = 180;
    }
    else
    {
      saturation = 255;
    }

    // early cut-off to black
    if (brightness <= 2)
    {
      brightness = 0;
    }
    
    leds[pixel] = CHSV(hue[pixel], saturation, brightness);
  }
}

void displayPixelState(int pixel)
{
  byte sourcePixel;
  if (linkedState == STATE_LINKED)
  {
    sourcePixel = 0;
  }
  else
  {
    sourcePixel = pixel;
  }
  
  switch (pixelState[sourcePixel])
  {
    case PIXEL_FADE_IN:
      {
        displayColor(pixel, 200 - index[sourcePixel]);
      }
      break;
    case PIXEL_FADE_OUT:
      {
        displayColor(pixel, index[sourcePixel]);
      }
      break;
  }
}

void updatePixelState(int pixel)
{
  index[pixel]--;
  if (index[pixel] >= 0)
  {
    switch (pixelState[pixel])
    {
      case PIXEL_FADE_IN:
      case PIXEL_FADE_OUT:
        {
          index[pixel]=index[pixel]-fadeRate;
          displayPixelState[pixel];
        }
        break;
    }
  }
  else
  {
    switch (pixelState[pixel])
    {
      case PIXEL_BLACK:
        {
          // if linked, must set ALL the colors, not just leds[0]
          if (linkedState == STATE_LINKED)
          {
            setHueAll();
          }
          else
          {
            setHue(pixel);
          }

          if (endingCounter > 0)
          {
            pixelState[pixel] = PIXEL_FADE_IN;
            index[pixel] = 200;
          }
          else
          {
            pixelState[pixel] = PIXEL_ENDING;
            index[pixel] = 0;
          }

          if (pixel == 0)
          {
            endingCounter--;
          }
        }
        break;
      case PIXEL_FADE_IN:
        {
          pixelState[pixel] = PIXEL_LIT;
          index[pixel] = random(litMin, litMax);
        }
        break;
      case PIXEL_LIT:
        {
          pixelState[pixel] = PIXEL_FADE_OUT;
          index[pixel] = 200;
        }
        break;
      case PIXEL_FADE_OUT:
        {
          pixelState[pixel] = PIXEL_BLACK;
          index[pixel] = random(darkMin, darkMax);
        }

        // decrement ending counter if pixel[0];
        if (pixel == 0)
        {
          endingCounter--;
        }
        
        break;
    }
  }
}


void handleStates()
{
  // display and update pixel states
  for(int i = 0; i < NUM_LEDS; i++)
  {
    displayPixelState(i);
    if ((linkedState != STATE_LINKED) || (i == 0))
    {
      updatePixelState(i);
    }
  }
  FastLED.show();

  // handle overall state change
  if (endingCounter <= 0)
  {
    int endedCount = 0;
    for(int i = 0; i < NUM_LEDS; i++)
    {
      if (linkedState == STATE_LINKED)
      {
        if (pixelState[0] == PIXEL_ENDING)
        {
          endedCount++;
        }
//        Serial.print("\npixelState[0]=");
//        Serial.print(pixelState[0]);
//        Serial.print("endedCount=");
//        Serial.print(endedCount);
      }
      else
      {
        if (pixelState[i] == PIXEL_ENDING)
        {
          endedCount++;
        }
//        Serial.print("\npixelState[");
//        Serial.print(i);
//        Serial.print("]=");
//        Serial.print(pixelState[i]);
//        Serial.print("endedCount=");
//        Serial.print(endedCount);
      }
    }



    if (endedCount == NUM_LEDS)
    {
      linkedState = random(2);
      colorState = random(3);

      if (linkedState == STATE_LINKED)
      {
        endingCounter = 1;
        litMin = 500;
        litMax = 1000;
        darkMin = 25;
        darkMax = 50;
        fadeRate = 1;
      }
      else
      {
        endingCounter = 8;
        litMin = 100;
        litMax = 300;
        darkMin = 100;
        darkMax = 300;
        fadeRate = random(1,5);
      }

      for(int i = 0; i < NUM_LEDS; i++)
      {
        pixelState[i] = PIXEL_BLACK;
      }
    }
  }
}


void serialInfo()
{
  Serial.print("\nloop: endingCounter=");
  Serial.print(endingCounter);
  Serial.print("\nloop: linkedState=");
  Serial.print(linkedState);
  Serial.print(", colorState=");
  Serial.print(colorState);
  Serial.print(", litMin=");
  Serial.print(litMin);
  Serial.print(", litMax=");
  Serial.print(litMax);
  Serial.print(", pixelState[0]=");
  Serial.print(pixelState[0]);
  Serial.print(", pixelState[1]=");
  Serial.print(pixelState[1]);
  Serial.print(", index[0]=");
  Serial.print(index[0]);
  Serial.print(", index[1]=");
  Serial.print(index[1]);
  Serial.print(", hue[0]=");
  Serial.print(hue[0]);
  Serial.print(", hue[1]=");
  Serial.print(hue[1]);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nresetting");
  FastLED.addLeds<WS2811,DATA_PIN,RGB>(leds,NUM_LEDS);
  linkedState = STATE_LINKED;
  pixelState[0] = PIXEL_ENDING;
  endingCounter = 0;
}
 
void loop() { 
  // put your main code here, to run repeatedly:
  // serialInfo();
  handleStates();
  delay(20);
}
