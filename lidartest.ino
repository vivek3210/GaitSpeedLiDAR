#include "LIDARLite_v4LED.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1327.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define OLED_RESET    -1  // No reset pin
Adafruit_SSD1327 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

LIDARLite_v4LED myLIDAR;
unsigned long startTime = 0;
unsigned long endTime = 0;
float startDistance = 0;
float endDistance = 0;
bool inRange = false;
const float distanceThreshold = 200.0;  // 2 meters in cm
const float interval = 200.0;           // 2 meters for gait measurement
const float distanceMax = 500.0;        // 5 meters in cm

const float maxReasonableSpeed = 4;
const float minReasonableSpeed = 0.3;

const int MAX_SPEEDS_STORED = 10;
int numStored = 0;
float recentSpeeds[MAX_SPEEDS_STORED];

void updateRecentSpeeds(float speed);
float getAvgOfStored();

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Qwiic LIDARLite_v4 Gait Measurement");
  Wire.begin();

  if (!myLIDAR.begin()) {
    Serial.println("Device did not acknowledge! Freezing.");
    while (1)
      ;
  }
  Serial.println("LIDAR acknowledged!");

  Serial.println("Scanning I2C...");
  for (byte address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      Serial.println(address, HEX);
    }
  }

  if (!display.begin(0x3C)) { // Check I2C address if needed
    Serial.println("OLED failed to initialize!");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1327_WHITE);
  display.setCursor(0, 0);
  display.println("Gait Sensor Ready");
  display.display();
  delay(2000);
}

void loop() {
  float distance = myLIDAR.getDistance();

  // if (distance <= distanceThreshold && distance <= distanceMax) {  // Object detected within 2 meters
  if (distance <= distanceMax) {
    if (!inRange) {                     // First time detection (starting point)
      startTime = millis();
      startDistance = distance;
      inRange = true;
      Serial.println();
      Serial.println("Object entered range, timing started.");
      Serial.print("starting distance: ");
      Serial.println(startDistance);

      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Tracking...");
      display.display();
      
    } else { // it is tracking you
      endDistance = distance;
      endTime = millis();
    }
  } else {          // Object moves out of range
    if (inRange) {  // End timing
      Serial.print("ending distance: ");
      Serial.println(endDistance);
      float timeElapsed = (endTime - startTime) / 1000.0;  // Convert to seconds
      Serial.print("time elapsed: ");
      Serial.println(timeElapsed);

      display.clearDisplay();
      display.print("Ending Distance: ");
      display.println(endDistance);
      display.print("Time Elapsed: ");
      display.println(endDistance);

      float speed = (endDistance - startDistance) / timeElapsed;                // Speed in m/s
      if (speed < 0) {
        speed = speed * -1;
      }

      speed = speed / 100;
      Serial.println();
      Serial.print("Gait Speed: ");
      Serial.print(speed);
      Serial.println(" m/s");
      
      display.println();
      display.print("Gait Speed: ");
      display.print(speed);
      display.println(" m/s");

      updateRecentSpeeds(speed);
      
      Serial.print("Average Gait Speed lately: ");
      Serial.println(getAvgOfStored());

      display.print("Average Gait Speed lately: ");
      display.println(getAvgOfStored());
      display.display();

      inRange = false;  // Reset for next detection
    }
  }


  delay(20);  // Prevent I2C overload
}

void updateRecentSpeeds(float mostRecentSpeed) {
  if (mostRecentSpeed > maxReasonableSpeed || mostRecentSpeed < minReasonableSpeed) {
    return;
  }

    for (int i = MAX_SPEEDS_STORED - 1; i > 0; i--) {
      recentSpeeds[i] = recentSpeeds[i-1];
    }
    recentSpeeds[0] = mostRecentSpeed;
    // Serial.print("supposedly most recent entry: ");
    // Serial.println(recentSpeeds[0]);

    if (numStored < MAX_SPEEDS_STORED) {
      numStored++;
      // Serial.print("num stored ");
      // Serial.println(numStored);
    }
}

float getAvgOfStored() {
  double total = 0;
  for (int i = 0; i < numStored; i++) {
    total += recentSpeeds[i];
  }
  // Serial.print("total ");
  // Serial.println(total);

  return (total / numStored);
}


