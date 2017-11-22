/*
   Code for controlling sound levels on Max based on
   proximity to a photoresistor/sensor.
*/

#include<Wire.h>

//-------- Arduino to Max --------//
#define LDRone A1
#define LDRtwo A2


//-------- Ultrasonic Sensor --------//
#define ledPin 6
int trigPin = 9;     //Trig - green Jumper
int echoPin = 10;    //Echo - yellow Jumper
long duration, cm, inches;

// median filter
const int numReadings = 5;

int readings[numReadings];
int sortedReading[numReadings];
int readIndex = 0;

int medianReading;
int oldReading = 10000;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LDRone, INPUT);
  pinMode(LDRtwo, INPUT);

  Wire.begin();

  // median filter: initialize all readings to 0
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
    sortedReading[thisReading] = 0;
  }
}

void loop() {
  //-------- Arduino to Max --------//
  Serial.println(analogRead(LDRone));  
  Serial.print("\r");
  Serial.println(analogRead(LDRtwo));
  //  following line uses much more memory
  //  (cause using String lib for the first time):
  //  Serial.println((String) lightLevelOne + "\t" + lightLevelTwo);

  //-------- Ultrasonic Sensor --------//
  
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:

  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);

  // convert the time into a distance
  cm = (duration / 2) / 29.1;
  inches = (duration / 2) / 74;

  delay(10);

  medianReading = medianReadings(cm);
  /*
  Serial.print(medianReading);
//  Serial.print("cm");
  Serial.println();
*/
  // LED STUFF

  if (medianReading <= 95 && abs(medianReading - oldReading) > 10) {
    Wire.beginTransmission(9);
    Wire.write(medianReading);
    Wire.endTransmission();
    digitalWrite(ledPin, HIGH);
 //   delay(100); // keep the light on for a while after detection
  } else {
    digitalWrite(ledPin, LOW);
  }

  oldReading = medianReading;
}

long medianReadings(long centimetres) {
  // store reading values into the array
  readings[readIndex] = centimetres;
  readIndex++;

  sort(readings, numReadings);

  int sortedReadings[numReadings];

  for (int i; i < 2; i++)
  {
    sortedReadings[i] = readings[i];

    //for debug
    //      Serial.print(i);
    //      Serial.print(": ");
    //      Serial.print(sortedReadings[i]);
    //      Serial.println();
  }

  // if we're at the end of the array
  if (readIndex >= numReadings) {
    // wrap around to the beginning
    readIndex = 0;
  }

  return sortedReadings[1];
  delay(1);
}

// bubble sort algorithm
void sort(int a[], int arraySize)
{
  for (int i = 0; i < (arraySize - 1); i++)
  {
    for (int j = 0; j < (arraySize - (i + 1)); j++)
    {
      if (a[j] > a[j + 1])
      {
        int t = a[j];
        a[j] = a[j + 1];
        a[j + 1] = t;
      }
    }
  }
}

