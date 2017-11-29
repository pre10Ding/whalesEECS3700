// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!

#include <Wire.h>
#include <Adafruit_NeoPixel.h>

//IDK WHAT THESE BUTTONS R FOR---------
#define BUTTON_PIN   2    // Digital IO pin connected to the button.  This will be
// driven with a pull-up resistor so the switch should
// pull the pin to ground momentarily.  On a high -> low
// transition the button press logic will execute.
#define BUTTON_PIN3 3
#define BUTTON_PIN4 4
#define BUTTON_PIN5 5
//------------------------------------


#define PHOTO_PIN A0

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

//to store colors
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

//again IDK WHAT THIS IS FOR--------------
#define BUTTON_COUNT 3
bool detectionButton3;
bool detectionButton4;
bool detectionButton5;
bool oldButton3 = HIGH;
//----------------------------------------

int restingR = 4;
int restingG = 4;
int restingB = 48;

int waves[BUTTON_COUNT] = {0};
int intensity2 = 0; //TEMPORTARY INTENSITY STORAGE IN reactiveLights()

bool ramp = false;
int flag = 0; //to track if all the LEDs reached their target color yet

int defaultBright = 32; //MODIFY THIS TO CHANGE TOTAL BRIFGHTNESS
int distance = 10000;
int oldDistance = 10000;
boolean ripple = false;

//Custom set color allowing for individual brightness control (UNUSED)
void setPixelColor( uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint16_t brightness) {
  strip.setPixelColor(n, (brightness * r / 255)>255? 255:(brightness * r / 255),
                         (brightness * g / 255)>255? 255:(brightness * g / 255), 
                         (brightness * b / 255)>255? 255:(brightness * b / 255));
}

void setup() {
//more legacy button code thats not used--------
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN3, INPUT_PULLUP);
  pinMode(BUTTON_PIN4, INPUT_PULLUP);
  pinMode(BUTTON_PIN5, INPUT_PULLUP);
  pinMode(PHOTO_PIN, INPUT);
//----------------------------------------------

//putting the LEDS in the strip into an array, add more rows for 2D array
  int row0[] = {0, 1, 2, 3, 4, 5, 6, 7};

  memcpy(twoDArrayLights[0], row0, COLS * sizeof(int));
  // memcpy(twoDArrayLights[1], row1, COLS * sizeof(int));

//Communication initiation with other Arduino
  Wire.begin(9);
  Wire.onReceive(receiveEvent);


  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Serial.begin(9600);

  for(int i = 0; i<PIXEL_COUNT; i++) {
    curLED[i].brightness = defaultBright;
  }
}


//This is triggered when the other arduino sends ultrasonic readings
void receiveEvent(int d) {
  distance = Wire.read();
  Serial.println(distance);
}


void loop() {
//more unused button stuff ---------------------------------
  detectionButton3 = digitalRead(BUTTON_PIN3);
  detectionButton4 = digitalRead(BUTTON_PIN4);
  detectionButton5 = digitalRead(BUTTON_PIN5);
//---------------------------------------------------------

  int pressedCoord[2]; //initializing the coordinate variable for storing the position of detection
  int lightLvl = analogRead(PHOTO_PIN)>200? 200: analogRead(PHOTO_PIN); //limiting photores reading from 0-200
  int intensityMain = 3 + (200-lightLvl)/50; //baseline intensity is 3, increases by every 50units of reading from photores
  //Serial.println(intensityMain);
  restingR = 4 + (200 - lightLvl)/5;
  

  pressedCoord[0] = 0;  //Manually set the coordinates of the detectors as if they are on the same grid as LEDs
  pressedCoord[1] = distance / 10; //Automatically setting the y coord by dividing the ultrasonic readings by 10
                                   //(how far from the sensor is the person)
  
  reactiveLights(20, pressedCoord, intensityMain); //call another method


  if (oldDistance != distance) { //if distance changed, trigger a ripple
    ripple = true;
  }
  oldDistance = distance;

}


void reactiveLights(uint8_t wait, int b[], int intensityMain) {

  for (int i = 0; i < PIXEL_COUNT; i++) {
    setPixelColor(i, restingR, restingG , restingB, defaultBright); //set a baseline color for all pixels
    // strip.setPixelColor(i, (random(5, 15), random(5, 15), random(45, 55))); //background blue colors,
  }

  if (ripple) { //WHen ripple is triggrd, give intensity to waves[]
    //so that it can be calculated and given to pixels
    intensity2 = intensityMain; /// TO BE CHANGED WITH INTENSITY INPUT
    waves[0] = intensity2; //TEMPORARY test of SINGLE ripple...

    setReactiveLightsHelper(intensity2, b);
    ripple = false;
    ramp = true;

    //need for loop to test the intensity left over for all the buttons
  } else if (!ramp && intensity2 > 0) {
    intensity2--;
    waves[0] = intensity2; //TEMPORARY test of SINGLE ripple...

    setReactiveLightsHelper(intensity2, b); //method that activates the recursive color determining function

    ramp = true;
  }
  if (ramp) { //this increments and decrements the colors til it hits target color
    for (int i = 0; i < PIXEL_COUNT; i++) {
      rampLights(i); // the ramping method call
    }

    //add 1 to flag for each LED that completed it's ramp
    for (int i = 0; i < PIXEL_COUNT; i++) {
      if (tarLED[i].down) {
        flag++;
      }
      if (flag == PIXEL_COUNT) { //when all LEDs completed ramp, stop ramping, reset flag
        ramp = false;
        for (int i = 0; i < PIXEL_COUNT; i++) {
          tarLED[i].down = false;
        }
      }
    }
    flag = 0;



//set all pixels to be the color determined in the curLED array 
    for (int i = 0; i < PIXEL_COUNT; i++) {
      //strip.setPixelColor(i, (curLED[i].r, curLED[i].g, curLED[i].b));
      setPixelColor(i, restingR, curLED[i].g, restingB, curLED[i].brightness);
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
  oldButton3 = detectionButton3; //old code ignore this
  
  delay(25); //smaller delay = faster ripples
}


void setReactiveLightsHelper(int intensity, int cord[]) { // activates the recursive function 
  for (int i = 0; i < 1; i++) { //MAGIC NUMBER CHANGE THIS LATER
 //   if (waves[i] > 0) {
      setReactiveLights(waves[i]--, cord, 0);

  //  }
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
    return; //if the coordinate is outside the boundies of the array of LEDs, return (end reached)
  }
  else {
    int i = intensity - 1; //decrement intensity, make the next one less bright
    
    if (d != 4) { //if the recursive call did NOT come from the right side
      int cord0[] = {cord[0] - 1, cord[1]};
      setReactiveLights(i, cord0, 2); //tell the next iteration that this is a call to the right 
    }
    if (d != 2) {
      int cord1[] = {cord[0] + 1, cord[1]};
      setReactiveLights(i, cord1, 4);
    }
    if (d != 1) {
      int cord2[] = {cord[0], cord[1] - 1};
      setReactiveLights(i, cord2, 3);
    }
    if (d != 3) {
      int cord3[] = {cord[0], cord[1] + 1};
      setReactiveLights(i, cord3, 1);
    }

    
    //test if there is a pixel at current coord
    int j = twoDArrayLights[cord[0]][cord[1]];
    /*   Serial.print(intensity);
       Serial.print(" - ");
       Serial.print(j);
       Serial.print("\n"); */
    if (j > -1 && j < PIXEL_COUNT) {
      //SET THE TARGET COLOR FOR EACH LED
      tarLED[j].r = (32 * intensity)>255? 255:32 * intensity ;
      tarLED[j].g = (32 * intensity)>255? 255:32 * intensity ;
      tarLED[j].b = (32 * intensity)>255? 255:32 * intensity ;
      
      tarLED[j].brightness = defaultBright * intensity;
/*
      Serial.print(j);
      Serial.print(" - ");
      Serial.print(tarLED[j].b);
      Serial.print("\n");
*/
      //   strip.setPixelColor(j, (random(10), random(10), random(25, 30)));
    }
  }
}
 

//TO RAMP LIGHTS ACCORIDNG TO DIF PEAK RGB, COMPARE FOR HIGHEST VALUE COMPONENT, HAVE IT CONTROL .down
void rampLights(int i) {
       Serial.println(curLED[2].g);

  if (!curLED[i].down) { //if the "down" flag is not true, ramp upwards, incrementing by 1/20th of the target RGB value each time
    if (tarLED[i].r > curLED[i].r) {
      curLED[i].r = curLED[i].r + tarLED[i].r / 20;
    }
    if (tarLED[i].g > curLED[i].g) {
      curLED[i].g = curLED[i].g + tarLED[i].g / 20;
  
    }
    if (tarLED[i].b > curLED[i].b) {
      curLED[i].b = (curLED[i].b + tarLED[i].b / 20)>255? 255: curLED[i].b + tarLED[i].b / 20;
    }// else curLED[i].down = true; //When current B value is larger than target B value, set "down" flag to true

    if(tarLED[i].brightness > curLED[i].brightness) {
      curLED[i].brightness += tarLED[i].brightness/20;
 //     curLED[i].g = (curLED[i].g + (tarLED[i].brightness)/60) > 60? 60: curLED[i].g + (tarLED[i].brightness/2)/20;

      
    } else curLED[i].down = true; //When current B value is larger than target B value, set "down" flag to true

    
  } else {
    if (curLED[i].r > restingR) { //if current is larger than the baseline R value, decrement by 1/20th of the target value
      curLED[i].r = curLED[i].r - tarLED[i].r / 20;
    } else {
      tarLED[i].r = restingR;   // else, set the value of both target and current to the baseline R value
      curLED[i].r = restingR;
    }
    if (curLED[i].g > restingG) {
      curLED[i].g = curLED[i].g - tarLED[i].g / 20;
    } else {
      tarLED[i].g = restingG;
      curLED[i].g = restingG;
    }
    if (curLED[i].b > restingB) {
      curLED[i].b = curLED[i].b - tarLED[i].b / 20;
    } /*else {
      tarLED[i].b = restingB;
      curLED[i].b = restingB; 
      curLED[i].down = false; //else set ramping "down" flag to false
      tarLED[i].down = true;  // tar "down" flag = true, which means the complete ramp cycle was completed
    }*/

    if(curLED[i].brightness > defaultBright) {
      curLED[i].brightness-=tarLED[i].brightness/20;
 //     curLED[i].g = (curLED[i].g - (tarLED[i].brightness)/60) > restingG? restingG: curLED[i].g - (tarLED[i].brightness/2)/20;

    }else {
      tarLED[i].brightness = defaultBright;
      curLED[i].brightness = defaultBright; 
      curLED[i].g = restingG;
      curLED[i].down = false; //else set ramping "down" flag to false
      tarLED[i].down = true;  // tar "down" flag = true, which means the complete ramp cycle was completed
    }
  }
}

