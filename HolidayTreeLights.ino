#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 300
#define NUM_LIGHTS 100
#define NUM_GLITTER 10

#define UNASSIGNED -1

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 52

#define COLOR_MODE_PASTEL              0
#define COLOR_MODE_RAINBOW             1
#define COLOR_MODE_WHITE               2
#define COLOR_MODE_CANDLE              3
#define COLOR_MODE_RAINBOW_GRADIENT    4

#define HUES 256
#define WARM_WHITE_HUE 30

#define COLORS 6
#define COLOR_RED     0
#define COLOR_ORANGE  1
#define COLOR_YELLOW  2
#define COLOR_GREEN   3
#define COLOR_BLUE    4
#define COLOR_PURPLE  5

#define MODES 3
#define MODE_FADE_OUT 0
#define MODE_SPARKLE 1
#define MODE_GLITTER 2
#define MODE_GRADIENT 3

#define MODE_FADE_START 2000
#define MODE_FADE_END 2050

// Define the array of leds
CRGB leds[NUM_LEDS];

int lightLed[NUM_LIGHTS];
CRGB lightRGB[NUM_LIGHTS];
byte lightDelay[NUM_LIGHTS];

byte mode, color, gradientHue, gradientStart;
int modeCounter, fadeRate;


byte getRainbowHue(byte color)
{
  byte hue;
  switch (color)
  {
    case COLOR_RED:    hue = HSVHue::HUE_RED;     break;
    case COLOR_ORANGE: hue = HSVHue::HUE_ORANGE;  break;
    case COLOR_YELLOW: hue = HSVHue::HUE_YELLOW;  break;
    case COLOR_GREEN:  hue = HSVHue::HUE_GREEN;   break;
    case COLOR_BLUE:   hue = HSVHue::HUE_BLUE;    break;
    case COLOR_PURPLE: hue = HSVHue::HUE_PURPLE;  break;
  }
  return hue;
}

byte getHue()
{
  byte hue;
  switch (color)
  {
    case COLOR_MODE_RAINBOW_GRADIENT:
      hue = gradientHue;
      gradientHue = gradientHue + 3;  //byte wraps around from 255 to 0 automatically
      break;
    case COLOR_MODE_PASTEL:
      hue = random(HUES);
      break;
    case COLOR_MODE_RAINBOW:
      hue = getRainbowHue(random(COLORS));
      break;
    case COLOR_MODE_WHITE:
    case COLOR_MODE_CANDLE:
      hue = WARM_WHITE_HUE;
      break;
  }
  return hue;
}

byte getSaturation()
{
  if (color == COLOR_MODE_WHITE)
    return 180;
  else
    return 255;
}

void getNewLights()
{
  for (int i = 0; i < NUM_LIGHTS; i++)
    getNewLight(i);
}

void getNewLight(byte index)
{
  boolean notFound = true;
  while (notFound)
  { 
    lightLed[index] = random(NUM_LEDS);
    notFound = false;
    for (int i = 0; i < NUM_LIGHTS; i++)
      if ((i != index) && (lightLed[i] == lightLed[index]))
        notFound = true;
  }
  lightDelay[index] = random(50, 256);
}

void setLeds()
{
  setLeds(0);
}

void setLeds(byte start)
{
  gradientHue = 0;
  int i = start + 1;
  if (i == NUM_LEDS)
    i = 0;
  while (i != start)
  {
    setLed(i);
    i++;
    if (i == NUM_LEDS)
      i = 0;
  }
}

void setLed(int index)
{
  leds[index] = CHSV(getHue(), getSaturation(), 255);
}

boolean isLit(int index)
{
  int rgbTotal = leds[index].r + leds[index].g + leds[index].b;
  if (rgbTotal > 0)
    return true;
  else
    return false;
}

boolean isWhite(int index)
{
  if ((leds[index].r == 255)
        && (leds[index].g == 255)
        && (leds[index].b == 255))
    return true;
  else
    return false;
}

void handleLight(byte index)
{
  if (lightDelay[index] > 0)
  {
    lightDelay[index]--;
  }
  else
  {
    switch (mode)
    {
      case MODE_FADE_OUT:
        if (isLit(lightLed[index]))
        {
          leds[lightLed[index]].fadeToBlackBy(fadeRate);
        }
        else
        {
          setLed(lightLed[index]);
          getNewLight(index);
        }
        break;
      case MODE_SPARKLE:
        if (isWhite(lightLed[index]))
        {
          leds[lightLed[index]] = lightRGB[index];
          getNewLight(index);
        }
        else
        {
          if (random(8) == 0)
          {
            lightRGB[index] = leds[lightLed[index]];
            leds[lightLed[index]] = CRGB(255, 255, 255);
          }
          else
            getNewLight(index);
        }
        break;
      case MODE_GLITTER:
        leds[lightLed[index]] = CHSV(getHue(), getSaturation(), 255);
        getNewLight(index);
        break;
    }
  }
}

void handleLights()
{
  for (int i = 0; i < NUM_LIGHTS; i++)
    handleLight(i);
}

void handleMode()
{
  if (modeCounter >= MODE_FADE_END)
  {
    modeCounter=0;
    byte lastMode = mode;
    byte lastColor = color;
    // pick a new mode
    while ((mode == lastMode) || (color == lastColor))
    {
      mode = random(MODES);

      if (mode == MODE_GRADIENT)
        color = COLOR_MODE_RAINBOW_GRADIENT;
      else
      {
        switch (random(7))
        {
          case 0:
          case 1:
          case 2:
            color = COLOR_MODE_RAINBOW;
            break;
          case 3:
          case 4:
          case 5:
            color = COLOR_MODE_WHITE;
            break;
          case 6:
            color = COLOR_MODE_PASTEL;
            break;
        }
      }
    }
    
    if (mode != MODE_GLITTER)
      if ((mode != MODE_FADE_OUT) || (random(2) == 0))
        setLeds();
    if (mode == MODE_GRADIENT)
      gradientStart = 0;
    else
      getNewLights();
    fadeRate = random(20,50);
  }
  else if (modeCounter >= MODE_FADE_START)
  {
    fadeToBlackBy(leds, NUM_LEDS, 20);
  }
  else
  {
    switch (mode)
    {
      case MODE_FADE_OUT:
      case MODE_SPARKLE:
        handleLights();
        break;
      case MODE_GLITTER:
        fadeToBlackBy(leds, NUM_LEDS, fadeRate);
        for (int i = 0; i < (NUM_GLITTER); i++)
          setLed(random(NUM_LEDS));
        break;
      case MODE_GRADIENT:
        setLeds(gradientStart);
        gradientStart++;
        if (gradientStart > NUM_LEDS)
          gradientStart = 0;
        break;
    }
  }
  FastLED.show();
  modeCounter++;
}


void serialInfo()
{
  Serial.print("\nloop: modeCounter=");
  Serial.print(modeCounter);
  Serial.print(", mode=");
  Serial.print(mode);
  Serial.print(", color=");
  Serial.print(color);
  Serial.print(", fadeRate=");
  Serial.print(fadeRate);
  Serial.print(", lightLed[0]=");
  Serial.print(lightLed[0]);
  Serial.print(", lightDelay[0]=");
  Serial.print(lightDelay[0]);
  Serial.print(", lightRGB[0]=");
  Serial.print(lightRGB[0].r);
  Serial.print("/");
  Serial.print(lightRGB[0].g);
  Serial.print("/");
  Serial.print(lightRGB[0].b);
  Serial.print(", lightLed[1]=");
  Serial.print(lightLed[1]);
  Serial.print(", lightDelay[1]=");
  Serial.print(lightDelay[1]);
  Serial.print(", lightRGB[1]=");
  Serial.print(lightRGB[1].r);
  Serial.print("/");
  Serial.print(lightRGB[1].g);
  Serial.print("/");
  Serial.print(lightRGB[1].b);
  Serial.print(", lightLed[2]=");
  Serial.print(lightLed[2]);
  Serial.print(", lightDelay[2]=");
  Serial.print(lightDelay[2]);
  Serial.print(", lightRGB[2]=");
  Serial.print(lightRGB[2].r);
  Serial.print("/");
  Serial.print(lightRGB[2].g);
  Serial.print("/");
  Serial.print(lightRGB[2].b);
  Serial.print(", lightLed[3]=");
  Serial.print(lightLed[3]);
  Serial.print(", lightDelay[3]=");
  Serial.print(lightDelay[3]);
  Serial.print(", lightRGB[2]=");
  Serial.print(lightRGB[3].r);
  Serial.print("/");
  Serial.print(lightRGB[3].g);
  Serial.print("/");
  Serial.print(lightRGB[3].b);
  Serial.print(", lightLed[4]=");
  Serial.print(lightLed[4]);
  Serial.print(", lightDelay[4]=");
  Serial.print(lightDelay[4]);
  Serial.print(", lightRGB[4]=");
  Serial.print(lightRGB[4].r);
  Serial.print("/");
  Serial.print(lightRGB[4].g);
  Serial.print("/");
  Serial.print(lightRGB[4].b);
  Serial.print(", gradientStart=");
  Serial.print(gradientStart);
  Serial.print(", leds[0]=");
  Serial.print(leds[0].r);
  Serial.print("/");
  Serial.print(leds[0].g);
  Serial.print("/");
  Serial.print(leds[0].b);
  Serial.print(", leds[1]=");
  Serial.print(leds[1].r);
  Serial.print("/");
  Serial.print(leds[1].g);
  Serial.print("/");
  Serial.print(leds[1].b);
  Serial.print(", leds[2]=");
  Serial.print(leds[2].r);
  Serial.print("/");
  Serial.print(leds[2].g);
  Serial.print("/");
  Serial.print(leds[2].b);
  Serial.print(", leds[3]=");
  Serial.print(leds[3].r);
  Serial.print("/");
  Serial.print(leds[3].g);
  Serial.print("/");
  Serial.print(leds[3].b);
  Serial.print(", leds[4]=");
  Serial.print(leds[4].r);
  Serial.print("/");
  Serial.print(leds[4].g);
  Serial.print("/");
  Serial.print(leds[4].b);
  
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nresetting");
  FastLED.addLeds<WS2811,DATA_PIN,RGB>(leds,NUM_LEDS);
  modeCounter = MODE_FADE_END;
}
 
void loop() { 
  // put your main code here, to run repeatedly:
  // serialInfo();
  handleMode();
  delay(20);
}
