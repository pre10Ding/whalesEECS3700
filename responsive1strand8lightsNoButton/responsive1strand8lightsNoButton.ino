// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!

#include <Wire.h>
#include <Adafruit_NeoPixel.h>

#define BUTTON_PIN   2    // Digital IO pin connected to the button.  This will be
// driven with a pull-up resistor so the switch should
// pull the pin to ground momentarily.  On a high -> low
// transition the button press logic will execute.
#define BUTTON_PIN3 3
#define BUTTON_PIN4 4
#define BUTTON_PIN5 5


#define PIXEL_PIN    6// Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 8

struct Adafruit_NeoPixel pixels[PIXEL_COUNT];

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800); // original
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800); //mine, trying to assign every pixel to an index in an array

typedef struct {
  int r;
  int g;
  int b;
  int brightness;
  bool down;
} targetLED;

targetLED tarLED[PIXEL_COUNT] = {0};
targetLED curLED[PIXEL_COUNT] = {0};

bool oldState = HIGH;
int showType = 0;
//int pixelArrEven[PIXEL_COUNT / 2];
//int pixelArrOdd[PIXEL_COUNT / 2];

//int inputLDR = A0;
//int lightLevel;

//2D array declaration
#define ROWS 1 //NUMBER OF ROWS OF LEDS
#define COLS 8

int twoDArrayLights[ROWS][COLS] = { -1};

#define BUTTON_COUNT 3
bool detectionButton3;
bool detectionButton4;
bool detectionButton5;
bool oldButton3 = HIGH;

int restingR = 4;
int restingG = 4;
int restingB = 8;

int waves[BUTTON_COUNT] = {0};
int intensity2 = 0; //TEMPORTARY INTENSITY STORAGE IN reactiveLights()

bool ramp = false;
int flag = 0; //to track if all the LEDs reached their target color yet

int brightness = 127; //MODIFY THIS TO CHANGE TOTAL BRIFGHTNESS
int distance = 10000;
int oldDistance = 10000;
boolean ripple = false;

//Custom set color allowing for individual brightness control
void setPixelColor( uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint16_t brightness) {
  strip.setPixelColor(n, (brightness*r/255) , (brightness*g/255), (brightness*b/255));
}

void setup() {
  // put your setup code here, to run once:
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN3, INPUT_PULLUP);
  pinMode(BUTTON_PIN4, INPUT_PULLUP);
  pinMode(BUTTON_PIN5, INPUT_PULLUP);

  int row0[] = {0, 1, 2, 3, 4, 5, 6, 7};
  /*
  int row0[] = {0, 1, 2, 3};
  int row1[] = {4, 5, 6, 7};
  int row2[] = {8, 9, 10, 11};
  int row3[] = {12, 13, 14, 15};
*/

  memcpy(twoDArrayLights[0], row0, COLS * sizeof(int));
  //memcpy(twoDArrayLights[1], row1, COLS * sizeof(int));
 // memcpy(twoDArrayLights[2], row1, COLS * sizeof(int));
 // memcpy(twoDArrayLights[3], row1, COLS * sizeof(int));

  strip.setBrightness(brightness);

  Wire.begin(9);
  Wire.onReceive(receiveEvent);

  strip.begin();
  //  Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB>(pixels,PIXEL_COUNT));
  strip.show(); // Initialize all pixels to 'off'
  Serial.begin(9600);
}

void receiveEvent(int d) {
  distance = Wire.read();
  Serial.println(distance);
}
void loop() {
  // put your main code here, to run repeatedly:

  detectionButton3 = digitalRead(BUTTON_PIN3);
  detectionButton4 = digitalRead(BUTTON_PIN4);
  detectionButton5 = digitalRead(BUTTON_PIN5);

  int pressedCoord[2];
  int intensityMain = 8;

//  if (detectionButton3) {
    pressedCoord[0] = 0;  //Manually set the coordinates of the detectors as if they are on the same grid as LEDs
    pressedCoord[1] = distance/10;
//  }
  reactiveLights(20, pressedCoord, intensityMain);

  if(oldDistance != distance) {
    ripple = true;
  }
  oldDistance = distance;

}


void reactiveLights(uint8_t wait, int b[], int intensityMain) {

  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, (3, 3, 10));
    // strip.setPixelColor(i, (random(5, 15), random(5, 15), random(45, 55))); //background blue colors,
  }

  if (ripple /*detectionButton3 == HIGH && oldButton3 == LOW */) { //WHen butten is pressed, give intensity to waves[]
                                                        //so that it can be calculated and given to pixels
    intensity2 = intensityMain; /// TO BE CHANGED WITH INTENSITY INPUT
    waves[0] = intensity2; //TEMPORARY test of button3...

    setReactiveLightsHelper(intensity2, b);
    ripple = false;
    ramp = true;
    //

    //need for loop to test the intensity left over for all the buttons
  } else if (!ramp && intensity2 > 0) {
    intensity2--;
    waves[0] = intensity2; //TEMPORARY test of button3...

    setReactiveLightsHelper(intensity2, b);

    ramp = true;
  } 
  if (ramp) { //this increments and decrements the colors til it hits target color
    for (int i = 0; i < PIXEL_COUNT; i++) {
      rampLights(i);
    }
    for (int i = 0; i < PIXEL_COUNT; i++) {
      if (tarLED[i].down) {
        flag++;
      }
      if (flag == PIXEL_COUNT) {
        ramp = false;
        for (int i = 0; i < PIXEL_COUNT; i++) {
          tarLED[i].down = false;
        }
      }
    }
    flag = 0;




    for (int i = 0; i < PIXEL_COUNT; i++) {
      strip.setPixelColor(i, (curLED[i].r, curLED[i].g, curLED[i].b));

    }
  }
 /* 
  Serial.print(curLED[2].b);
  Serial.print(" - ");
  Serial.print(ramp);
  Serial.print(" - ");
  Serial.print(intensity2);
  Serial.print("\n");
  */
  strip.show();
  oldButton3 = detectionButton3;
  delay(20);
}

void setReactiveLightsHelper(int intensity, int cord[]) {
  for (int i = 0; i < BUTTON_COUNT; i++) {
    if (waves[i] > 0) {
      setReactiveLights(waves[i]--, cord, 0);

    }
  }

}

//Direction the recursion is passed 
// 0 = first
// 1 = up
// 2 = right
// 3 = down
// 4 = left

void setReactiveLights(int intensity, int cord[], int d) { //Take in the intensity and the coordinate of the button pushed
  if (intensity < 1 || cord[0] < 0 || cord[1] < 0 || cord[0] >= ROWS || cord[1] >= COLS) { //base case
    return;
  }
  else {
    int i = intensity - 1;
    if(d != 4) {
      int cord0[] = {cord[0] - 1, cord[1]};
      setReactiveLights(i, cord0, 2);
    }
    if(d != 2) {
      int cord1[] = {cord[0] + 1, cord[1]};
      setReactiveLights(i, cord1, 4);
    }
    if(d != 1) {
      int cord2[] = {cord[0], cord[1] - 1};
      setReactiveLights(i, cord2, 3);
    }
    if(d != 3) {
      int cord3[] = {cord[0], cord[1] + 1};
      setReactiveLights(i, cord3, 1);
    }
    //test if there is a pixel at current coord
    int j = twoDArrayLights[cord[0]][cord[1]];
    /*   Serial.print(intensity);
       Serial.print(" - ");
       Serial.print(j);
       Serial.print("\n"); */
    if (j > -1&&j<PIXEL_COUNT) {
      //SET THE TARGET COLOR FOR EACH LED
      tarLED[j].r = 16 * intensity;
      tarLED[j].g = 16 * intensity;
      tarLED[j].b = 32 * intensity;
     
      Serial.print(j);
      Serial.print(" - ");
      Serial.print(tarLED[j].b);
      Serial.print("\n");
  
      //   strip.setPixelColor(j, (random(10), random(10), random(25, 30)));
    }
  }
}

void rampLights(int i) {
  if (!curLED[i].down) {
        if (tarLED[i].r > curLED[i].r) {
          curLED[i].r = curLED[i].r + tarLED[i].r / 20;

        }
        if (tarLED[i].g > curLED[i].g) {
          curLED[i].g = curLED[i].g + tarLED[i].g / 20;
        }
        if (tarLED[i].b > curLED[i].b) {
          curLED[i].b = curLED[i].b + tarLED[i].b / 20;
        } else curLED[i].down = true;
      } else {
        if (curLED[i].r > restingR) {
          curLED[i].r = curLED[i].r - tarLED[i].r / 20;
        } else {
          tarLED[i].r = restingR;

        }
        if (curLED[i].g > restingG) {
          curLED[i].g = curLED[i].g - tarLED[i].g / 20;
        } else {
          tarLED[i].g = restingG;

        }
        if (curLED[i].b > restingB) {
          curLED[i].b = curLED[i].b - tarLED[i].b / 20;
        } else {
          tarLED[i].b = restingB;
          curLED[i].down = false;
          tarLED[i].down = true;
        }
      }
}

