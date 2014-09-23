#include <Adafruit_NeoPixel.h>

#define PIN                  9  // Data output pin on the arduino
#define BUTTON_PIN           8  // Data input pin for the button, -1 to disable
#define PIR_PIN              7  // Data input pin for the pir, -1 to disable
#define PIXEL_COUNT         16  // Number of neo-pixels being controlled
#define COLOR_COUNT         12  // Number of pre-defined colors in colorMap
#define MIN_FADE_ALPHA       5  // Minimum brightness to use during fade in/out
#define MAX_FADE_ALPHA     250  // Maximum brightness to use during fade in/out
#define MIN_PULSE_ALPHA     30  // Minimum brightness to use during pulse
#define MAX_PULSE_ALPHA    250  // Maximum brightness to use during pulse
#define ALPHA_STEP           5  // The amount to change the brightness by each loop
#define FADE_STEP_DELAY     30  // Number of milliseconds to pause between steps for fade
#define PULSE_STEP_DELAY    30  // Number of milliseconds to pause between steps for pulse
#define MODE_DELAY        1000  // Number of milliseconds to pause between mode changes
#define PULSE_DELAY        500  // Number of milliseconds to pause between pulses
#define PULSE_MAX            2  // The number of pulses to complete before returning to fading
#define DEBOUNCE_DELAY      50  // The debounce time for the button
#define PIR_ACTIVE_TIME    150  // The time to remain active after motion is sensed (in seconds)
#define RANDOMIZED_COLORS    1  // Randomize the next color in the sequence
#define RANDOMIZED_START     0  // Randomize the first color chosen
#define DEBUG                0  // 1 for serial debugging output, 0 to disable


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_COUNT, PIN);

uint8_t       colorIdx            =              0,  // Start red (unless random)
              mode                =              0,  // The current mode, 1 is fade in, 2 is pulse, 3 is fade out
              pulseCount          =              0,  // The current count of pulses for mode 2
              alpha               = MIN_FADE_ALPHA;  // The current alpha level, used during fade in/out
int           lastButtonState     =            LOW,  // The previous reading from the input pin
              buttonState         =            LOW,  // The current reading from the input pin
              manualMode          =              0,  // 1 for manual mode, 0 for automatic mode
              manualModeColorIdx  =              0,  // Color index for manual mode
              manualModeUpdateReq =              1,  // 1 if update is required, 0 otherwise
              isPaused            =              0,  // Whether we are paused due to no movement
              curPulseAlphaStep   =              0,  // The current pulse alpha step, positive or negative
              modeIsChanging      =              0;  // Whether the mode is changing in the delay loop
unsigned long lastDebounceTime    =              0,  // The last time the output pin was toggled
              lastMovementTime    =              0,  // The last time movement was sensed
              currentDelay        =              0,  // The current delay we're waiting for
              lastDelayStartTime  =              0;  // The time the last delay was started

const uint32_t colorMap[COLOR_COUNT] = {
  0xFF0000,  //  Red
  0xFF7F00,  //  Orange
  0xFFFF00,  //  Yellow
  0x7FFF00,  //  Spring Grass
  0x00FF00,  //  Green
  0x00FF7F,  //  Turquoise
  0x00FFFF,  //  Cyan
  0x007FFF,  //  Ocean
  0x0000FF,  //  Blue
  0x7F00FF,  //  Violet
  0xFF00FF,  //  Magenta
  0xFF007F   //  Raspberry
};

void setup() {
  if (DEBUG) {
    Serial.begin(9600);
  }
  debug("Skull Debugging");

  if (BUTTON_PIN >= 0) {
    pinMode(BUTTON_PIN, INPUT);
  }
  if (PIR_PIN >= 0) {
    pinMode(PIR_PIN, INPUT);
    lastMovementTime = millis();
  }

  if (RANDOMIZED_COLORS || RANDOMIZED_START) {
    randomSeed(analogRead(0));
    colorIdx = random(COLOR_COUNT);
  }

  pixels.begin();
  pixels.setBrightness(MIN_FADE_ALPHA); // Just as a starting point
  debug("Setting brightness ", MIN_FADE_ALPHA);
}

void loop() {

  checkSensors();



  if (manualMode) {
    if (manualModeUpdateReq) {
        debug("Manual Mode set colors");

        for (uint8_t pixelIdx = 0; pixelIdx < PIXEL_COUNT; pixelIdx++) {
          pixels.setPixelColor(pixelIdx, colorMap[manualModeColorIdx]);
        }
        debug("Setting colors ", manualModeColorIdx);

        pixels.setBrightness(MAX_FADE_ALPHA);
        pixels.show();
        debug("Setting brightness ", MAX_FADE_ALPHA);

        manualModeUpdateReq = 0;
    }

  } else if (!isPaused) {
    unsigned long currentMillis = millis();

    if (currentMillis - lastDelayStartTime >= currentDelay) {

      switch(mode) {
        case 0: // Fade In
          if (alpha == MIN_FADE_ALPHA) {
            // Fade in also sets a new color.  Other modes keep the same color.
            for (uint8_t pixelIdx = 0; pixelIdx < PIXEL_COUNT; pixelIdx++) {
              pixels.setPixelColor(pixelIdx, colorMap[colorIdx]);
            }
            debug("Setting colors ", colorIdx);
          }
    
          if (!modeIsChanging) {
            if (alpha <= MAX_FADE_ALPHA) {
              pixels.setBrightness(alpha);
              debug("Setting brightness ", alpha);
              pixels.show();
  
              currentDelay = FADE_STEP_DELAY;
              alpha += ALPHA_STEP;

            } else {
              currentDelay = MODE_DELAY;
              modeIsChanging = 1;
            }

          } else {
            modeIsChanging = 0;
            alpha = MAX_PULSE_ALPHA;
            currentDelay = 0;
            curPulseAlphaStep = ALPHA_STEP * -1;
            mode++;
          }
    
          break;
  
       case 1: // Pulse

          if (!modeIsChanging) {
            if (pulseCount < PULSE_MAX) {
              if (curPulseAlphaStep < 0 && alpha >= MIN_PULSE_ALPHA) {
                pixels.setBrightness(alpha);
                debug("Setting brightness ", alpha);
                pixels.show();
    
                currentDelay = PULSE_STEP_DELAY;
                alpha += curPulseAlphaStep;
  
              } else if (curPulseAlphaStep < 0 && alpha < MIN_PULSE_ALPHA) {
                currentDelay = PULSE_DELAY;
                curPulseAlphaStep *= -1;
                alpha = MIN_PULSE_ALPHA;
  
              } else if (curPulseAlphaStep > 0 && alpha <= MAX_PULSE_ALPHA) {
                pixels.setBrightness(alpha);
                debug("Setting brightness ", alpha);
                pixels.show();
    
                currentDelay = PULSE_STEP_DELAY;
                alpha += ALPHA_STEP;
  
              } else if (curPulseAlphaStep > 0 && alpha > MAX_PULSE_ALPHA) {
                currentDelay = PULSE_DELAY;
                curPulseAlphaStep *= -1;
                alpha = MAX_PULSE_ALPHA;
                pulseCount++;

              }
            } else {
              currentDelay = MODE_DELAY;
              modeIsChanging = 1;
            }

          } else {
            modeIsChanging = 0;
            alpha = MAX_FADE_ALPHA;
            pulseCount = 0;
            currentDelay = 0;
            mode++;
          }

          break;
  
        case 2:  // Fade out
  
          if (!modeIsChanging) {
            if (alpha >= MIN_FADE_ALPHA) {
              pixels.setBrightness(alpha);
              debug("Setting brightness ", alpha);
              pixels.show();
  
              currentDelay = FADE_STEP_DELAY;
              alpha -= ALPHA_STEP;

            } else {
              currentDelay = MODE_DELAY;
              modeIsChanging = 1;
            }

          } else {
            modeIsChanging = 0;
            alpha = MIN_FADE_ALPHA;
            currentDelay = 0;
            mode = 0;
            if (RANDOMIZED_COLORS) {
              colorIdx = random(COLOR_COUNT);
            } else {
              colorIdx++;
              if (colorIdx >= COLOR_COUNT) {
                colorIdx = 0;
              }
            }
          }

          break;
      }

      lastDelayStartTime = millis();
    }
  }
}

int checkSensors() {
  int changeReported = 0;

  if (checkButton()) {
    changeReported = 1;
  }

  if (checkPir()) {
    changeReported = 1;
  }

  return changeReported;
}

int checkButton() {
  int changed = 0;

  if (BUTTON_PIN >= 0) {
    int reading = digitalRead(BUTTON_PIN);
  
    if (reading != lastButtonState) {
      lastDebounceTime = millis();
    }
  
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
      if (reading != buttonState) {
        buttonState = reading;
  
        if (buttonState == HIGH) {
          changed = 1;
          if (manualMode) {
            manualModeColorIdx++;
            manualModeUpdateReq = 1;
            if (manualModeColorIdx >= COLOR_COUNT) {
              //  Reset everything and go back to auotmatic mode
              manualModeColorIdx = 0;
              manualMode = 0;
              if (RANDOMIZED_COLORS || RANDOMIZED_START) {
                colorIdx = random(COLOR_COUNT);
              } else {
                colorIdx = 0;
              }
              mode = 0;
              isPaused = 0;
              modeIsChanging = 0;
              alpha = MIN_FADE_ALPHA;
              currentDelay = 0;
              lastMovementTime = millis();
  
              debug("Turning off manual mode");
            }
          } else {
            //  Change to manual mode.  No need to reset color index, it should already be reset
            manualMode = 1;
  
            debug("Turning on manual mode");
          }
        }
      }
    }
  
    lastButtonState = reading;
  }

  return changed;
}

int checkPir() {
  int changed = 0;

  if (PIR_PIN >= 0 && !manualMode) {
    int reading = digitalRead(PIR_PIN);

    if (reading) {
      lastMovementTime = millis();

      if (isPaused) {
        //  Reset the variables that make sense and unpause
        isPaused = 0;
        manualModeUpdateReq = 1;
        mode = 0;
        modeIsChanging = 0;
        alpha = MIN_FADE_ALPHA;
        currentDelay = 0;
        changed = 1;

        debug("Unpaused");
      }

    } else {
      if (!isPaused) {
        debug("Seconds since last movement: ", lround((millis() - lastMovementTime) / 1000));
        if (lround((millis() - lastMovementTime) / 1000) > PIR_ACTIVE_TIME) {

          //  Timeout reached.  Pause the display
          isPaused = 1;
          changed = 1;

          for (uint8_t pixelIdx = 0; pixelIdx < PIXEL_COUNT; pixelIdx++) {
            pixels.setPixelColor(pixelIdx, 0x000000);
          }
          debug("Setting colors off");
          pixels.show();

          debug("Paused");
        }
      }

    }
  }

  return changed;
}

void debug(char* string) {
  if (DEBUG) {
    Serial.println(string);
  }
}

void debug(char* string, int number) {
  if (DEBUG) {
    Serial.print(string);
    Serial.println(number);
  }
}

