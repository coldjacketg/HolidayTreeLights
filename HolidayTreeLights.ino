#include <HID.h>

#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 300
#define NUM_LIGHTS 50
#define NUM_BAND_SIZE 50

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 52

#define COLOR_MODE_RAINBOW  0
#define COLOR_MODE_WHITE    1
#define COLOR_MODE_PASTEL   2

#define HUES 256
#define WARM_WHITE_HUE 30
#define WARM_WHITE_SATURATION 180

#define COLORS 6
#define COLOR_RED     0
#define COLOR_ORANGE  1
#define COLOR_YELLOW  2
#define COLOR_GREEN   3
#define COLOR_BLUE    4
#define COLOR_PURPLE  5

#define MODES 5  // don't like candle in this version, so skip it
#define MODE_FADE 0
#define MODE_SPARKLE 1
#define MODE_GLITTER 2
#define MODE_GLITTER_WAVE 3
#define MODE_GRADIENT 4
#define MODE_CANDLE 5

// Define the array of leds
CRGB leds[NUM_LEDS];

int lightLed[NUM_LIGHTS];
CRGB lightRGB[NUM_LIGHTS];
byte lightDelay[NUM_LIGHTS];

CRGBPalette16 candlePalette;

byte mode, color, fadeRate, glitterChance, gradientStart;
int endingCounter;

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
    case COLOR_MODE_RAINBOW:
      hue = getRainbowHue(random(COLORS));
      break;
    case COLOR_MODE_WHITE:
      hue = WARM_WHITE_HUE;
      break;
    case COLOR_MODE_PASTEL:
      hue = random(HUES);
      break;
  }
  return hue;
}

byte getSaturation()
{
  byte saturation;
  switch (color)
  {
    case COLOR_MODE_WHITE:
      saturation = WARM_WHITE_SATURATION;
      break;
    default:
      saturation = 255;
      break;
  }
  return saturation;
}

void getNewLights()
{
  for (int i = 0; i < NUM_LIGHTS; i++)
    getNewLight(i);
}

void getNewLight(byte light)
{
  boolean notFound = true;
  while (notFound)
  { 
    lightLed[light] = random(NUM_LEDS);
    notFound = false;
    for (int i = 0; i < NUM_LIGHTS; i++)
      if ((i != light) && (lightLed[i] == lightLed[light]))
        notFound = true;
  }
  lightDelay[light] = random(50, 256);
}

void setLeds()
{
  for (int i=0; i<NUM_LEDS; i++)
    setLed(i);
}

void setLed(int led)
{
  if (((mode != MODE_GLITTER) && (mode != MODE_GLITTER_WAVE)) || (random(100) < glitterChance))
    leds[led] = CHSV(getHue(), getSaturation(), 255);
}

boolean isLit(int led)
{
  int rgbTotal = leds[led].r + leds[led].g + leds[led].b;
  if (rgbTotal > 0)
    return true;
  else
    return false;
}

boolean isWhite(int led)
{
  if ((leds[led].r == 255)
        && (leds[led].g == 255)
        && (leds[led].b == 255))
    return true;
  else
    return false;
}

void setCandle(byte light)
{
  byte paletteIndex = lightRGB[light].r;  // use RGB red channel for candle palette hue
  paletteIndex = paletteIndex + random(50);
  lightRGB[light].r = paletteIndex;
  leds[lightLed[light]] = ColorFromPalette(candlePalette, paletteIndex);
}


void handleLight(byte light)
{
  if (lightDelay[light] > 0)
  {
    lightDelay[light]--;
    if (mode == MODE_CANDLE)
      setCandle(light);
  }
  else
  {
    switch (mode)
    {
      case MODE_FADE:
      case MODE_CANDLE:
        if (isLit(lightLed[light]))
          leds[lightLed[light]].fadeToBlackBy(fadeRate);
        else
        {
          if (mode != MODE_CANDLE)
            setLed(lightLed[light]);
          getNewLight(light);
        }
        break;
      case MODE_SPARKLE:
        if (isWhite(lightLed[light]))
        {
          leds[lightLed[light]] = lightRGB[light];
          getNewLight(light);
        }
        else
        {
          if (random(3) == 0)  // only 1/3 of lights will trigger a strobe
          {
            lightRGB[light] = leds[lightLed[light]];
            leds[lightLed[light]] = CRGB(255, 255, 255);
          }
          else
            getNewLight(light);
        }
        break;
      case MODE_GLITTER:
        leds[lightLed[light]] = CHSV(getHue(), getSaturation(), 255);
        getNewLight(light);
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
  if (endingCounter == 1)
  {
    endingCounter=0;
    byte lastMode = mode;
    byte lastColor = color;
    // pick a new mode
    while ((mode == lastMode) || (color == lastColor))
    {
      mode = random(MODES);

      switch (random(10))
      {
        case 0:
        case 1:
        case 2:
        case 3:
          color = COLOR_MODE_RAINBOW;
          break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
          color = COLOR_MODE_WHITE;
          break;
        case 9:
          color = COLOR_MODE_PASTEL;
          break;
      }
    }

    switch (mode)
    {
      case MODE_GRADIENT:
        switch (random(7))
          {
            // rainbow
            case 0:
              color = COLOR_MODE_RAINBOW;
              break;
            case 1:  // red/white
              color = COLOR_MODE_WHITE;
              lightRGB[0] = CHSV(HSVHue::HUE_RED, 255, 255);
              lightRGB[1] = CHSV(WARM_WHITE_HUE, WARM_WHITE_SATURATION, 255);
              break;
            case 2:  // red/green
              color = COLOR_MODE_WHITE;
              lightRGB[0] = CHSV(HSVHue::HUE_RED, 255, 255);
              lightRGB[1] = CHSV(HSVHue::HUE_GREEN, 255, 255);
              break;
            case 3:  // blue/white
              color = COLOR_MODE_WHITE;
              lightRGB[0] = CHSV(HSVHue::HUE_BLUE, 255, 255);
              lightRGB[1] = CHSV(WARM_WHITE_HUE, WARM_WHITE_SATURATION, 255);
              break;
            case 4:  // green/white
              color = COLOR_MODE_WHITE;
              lightRGB[0] = CHSV(HSVHue::HUE_GREEN, 255, 255);
              lightRGB[1] = CHSV(WARM_WHITE_HUE, WARM_WHITE_SATURATION, 255);
              break;
            case 5:  // black/white
              color = COLOR_MODE_WHITE;
              lightRGB[0] = CRGB(0, 0, 0);
              lightRGB[1] = CHSV(WARM_WHITE_HUE, WARM_WHITE_SATURATION, 255);
              break;
            case 6:  // orange/purple
              color = COLOR_MODE_WHITE;
              lightRGB[0] = CHSV(HSVHue::HUE_ORANGE, 255, 255);
              lightRGB[1] = CHSV(HSVHue::HUE_PURPLE, 255, 255);
              break;
          }
          gradientStart = 0;
          break;
        case MODE_GLITTER:
        case MODE_GLITTER_WAVE:
          gradientStart = 0;
          if (color == COLOR_MODE_WHITE)
            glitterChance = random(5, 15);
          else
            glitterChance = random(1, 4);
          if (mode == MODE_GLITTER_WAVE)
            glitterChance = glitterChance * 2;
          break;
        case MODE_CANDLE:
          getNewLights();
          break;
        default:
          if ((mode != MODE_FADE) || (random(2) == 0))
            setLeds();
          else
            getNewLights();
          break;
    }
    fadeRate = random(20,50);
  }
  else if (endingCounter > 1)
  {
    fadeToBlackBy(leds, NUM_LEDS, 20);
    endingCounter--;
  }
  else
  {
    switch (mode)
    {
      case MODE_GRADIENT:
        if (color == COLOR_MODE_RAINBOW)  // rainbow
        {
          fill_rainbow(leds, NUM_LEDS, gradientStart, 3);
          gradientStart = gradientStart + 3;  // rolls over to 0 automatically
        }
        else
        {
          int band = -2;  // must shift two full bands to both colors
          while (((band) * NUM_BAND_SIZE) < NUM_LEDS)
          {
            band++;
            int bandStart = ((band - 1) * NUM_BAND_SIZE) + gradientStart;
            int bandEnd = (band * NUM_BAND_SIZE) + gradientStart - 1;
            if ((bandStart < 0) && (bandEnd < 0))
              continue;
            if ((bandStart >= NUM_LEDS) && (bandEnd >= NUM_LEDS))
              continue;
            if (bandStart < 0)
              bandStart = 0;
            if (bandStart >= NUM_LEDS)
              bandStart = NUM_LEDS - 1;
            if (bandEnd < 0)
              bandEnd = 0;
            if (bandEnd >= NUM_LEDS)
              bandEnd = NUM_LEDS - 1;
            if (band%2 == 0)
              fill_solid(&leds[bandStart], bandEnd - bandStart, lightRGB[0]);
            else
              fill_solid(&leds[bandStart], bandEnd - bandStart, lightRGB[1]);
          }
          gradientStart = gradientStart + 3;
          if (gradientStart >= (NUM_BAND_SIZE*2))  // must shift two full bands to both colors
            gradientStart = 0;
        }
        break;
      case MODE_FADE:
      case MODE_SPARKLE:
      case MODE_CANDLE:
        handleLights();
        break;
      case MODE_GLITTER:
        fadeToBlackBy(leds, NUM_LEDS, fadeRate);
        setLeds();
        break;
      case MODE_GLITTER_WAVE:
        fadeToBlackBy(leds, NUM_LEDS, fadeRate);
        int bandEnd = NUM_LEDS - gradientStart - 1 + NUM_BAND_SIZE;
        int bandStart = NUM_LEDS - gradientStart - 1;
        if (bandStart < 0)
          bandStart = 0;
        if (bandStart >= NUM_LEDS)
          bandStart = NUM_LEDS - 1;
        if (bandEnd < 0)
          bandEnd = 0;
        if (bandEnd >= NUM_LEDS)
          bandEnd = NUM_LEDS - 1;
        for (int i = bandStart; i <= bandEnd; i++)
          setLed(i);
        gradientStart++;
        if (gradientStart >= (NUM_LEDS + NUM_BAND_SIZE - 1))
          gradientStart = 0;
        break;
    }
  }
  FastLED.show();
}


void serialInfo()
{
  Serial.print("\nloop: endingCounter=");
  Serial.print(endingCounter);
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
  randomSeed(analogRead(0));
  FastLED.addLeds<WS2811,DATA_PIN,RGB>(leds,NUM_LEDS);
  candlePalette = CRGBPalette16( CRGB::DarkOrange, CRGB::Orange, CRGB::DarkOrange, CRGB::SandyBrown);
  endingCounter = 50;
}
 
void loop() { 
  // put your main code here, to run repeatedly:
  // serialInfo();
  handleMode();
  EVERY_N_SECONDS(30) { endingCounter = 50; }
  delay(20);
}
