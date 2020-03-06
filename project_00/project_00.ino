#include <FastLED.h>
#include <M5Stack.h>

#include "AudioFileSourceSD.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

#define SINGLE_PIN 2
#define RING_PIN 5

#define SINGLE_LED 1
#define RING_LEDS 16

CRGB single[SINGLE_LED];
CRGB ring[RING_LEDS];
int count_switch = 0;

AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2S *out;
AudioFileSourceID3 *id3;

bool on_flag = false;
float base_vol = 0;
int count_led = 0;
float sensitivity = 4.f;
float max_brightness = 50.f;
float rad_a = 0.f;
float rad_b = 0.f;

void playMP3(char *filename) {
  file = new AudioFileSourceSD(filename);
  id3 = new AudioFileSourceID3(file);
  out = new AudioOutputI2S(0, 1);
  out->SetOutputModeMono(true);
  out->SetGain(1.0);
  mp3 = new AudioGeneratorMP3();
  mp3->begin(id3, out);
  while (mp3->isRunning()) {
    if (!mp3->loop()) mp3->stop();
  }
}

void yuragi(bool flag) {
  float brightness = max_brightness * 0.5f * ((0.75f + (0.25f * sin(rad_a))) + (0.75f + (0.25f * cos(rad_b))));
  FastLED.setBrightness(int(brightness));

  float target_val = 0.5f + sensitivity * (base_vol);

  if (target_val > 1.5f) {
    base_vol += 0.005f;
  } else if (target_val < 0.5f) {
    base_vol -= 0.005f;
  }

  int red = 95.6f + 104.f * target_val + (-21.f * pow(target_val, 2)) + (1.25f * pow(target_val, 3));
  int green = 56.3f + 161.f * target_val + (-42.4f * pow(target_val, 2)) + (2.81f * pow(target_val, 3));
  int blue = 13.8f + 490.f * target_val + (-322.f * pow(target_val, 2)) + (68.5f * pow(target_val, 3)) + (-4.73f * pow(target_val, 4));
  if (red > 255) {
    red = 255;
  } else if (red < 0) {
    red = 0;
  }
  if (green > 255) {
    green = 255;
  } else if (green < 0) {
    green = 0;
  }
  if (blue > 255) {
    blue = 255;
  } else if (blue < 0) {
    blue = 0;
  }

  for (int i = 0; i < RING_LEDS; i++) {
    ring[i].setRGB(red, green, blue);
  }
  FastLED.show();
  count_led++;
  rad_a += 0.035f;
  rad_b += 0.021f;
}

void led_reset() {
  for (int i = 0; i < SINGLE_LED; i++) {
    single[i] = CRGB(0, 0, 0);
    FastLED.show();
  }
  for (int i = 0; i < RING_LEDS; i++) {
    ring[i] = CRGB(0, 0, 0);
    FastLED.show();
  }
}

void setup() {
  M5.begin();
  M5.Power.begin();

  FastLED.addLeds<NEOPIXEL, RING_PIN>(ring, RING_LEDS);
  FastLED.addLeds<NEOPIXEL, SINGLE_PIN>(single, SINGLE_LED);

  led_reset();

  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(3, 10);
  M5.Lcd.println("Pless A to change LED");
  M5.Lcd.setCursor(3, 35);
  M5.Lcd.println("Press B to Speak");
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    if (count_switch == 0) {
      M5.Lcd.printf("count 0 NOMAL \r\n");
      led_reset();
      on_flag = true;
      count_switch++;
    }
    else if (count_switch == 1) {
      M5.Lcd.printf("count 1 BLINKER \r\n");
      led_reset();
      on_flag = false;
      for (int i = 0; i < SINGLE_LED; i++) {
        single[i] = CRGB(255, 255, 255);
        FastLED.show();
      }
      count_switch++;
    }
    else if (count_switch == 2) {
      M5.Lcd.printf("count 2 HELP \r\n");
      led_reset();
      int num_blink = 3;
      for (int j = 0; j < num_blink; j++) {
        for (int i = 0; i < RING_LEDS; i++) {
          ring[i] = CRGB(255, 30, 0);
          FastLED.show();
        }
        delay(300);
        for (int i = 0; i < RING_LEDS; i++) {
          ring[i] = CRGB(0, 0, 0);
          FastLED.show();
        }
        delay(300);
      }
      count_switch++;
    }
    else if (count_switch == 3) {
      M5.Lcd.printf("count 3 GO \r\n");
      led_reset();
      for (int i = 0; i < RING_LEDS; i++) {
        ring[i] = CRGB(0, 100, 255);
        FastLED.show();
      }
      count_switch = 0;
    }
  }

  if (on_flag) {
    yuragi(on_flag);
  }

  if (M5.BtnB.wasPressed())
  {
    M5.Lcd.printf("SPEAK \r\n");
    playMP3("/hellow.mp3");
    delay(10);
  }
}
