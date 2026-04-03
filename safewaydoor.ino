#include <Servo.h>
#include "Adafruit_VL53L0X.h"

Servo doorServo_left;
Servo doorServo_right;

Adafruit_VL53L0X sensor_front = Adafruit_VL53L0X();

// servos for the door 
const int servoPin_left = 13;
const int servoPin_right = 12;

// misc variables
const int maxDistance = 100; // The VL53L0X works in mm, so 200mm = 20cm
const int minDistance = 20;  // 20mm = 2cm

bool isOpen = false;      // current state of the door
int zeroReading = 0;      // useful to ensure nothing is truly there infront of the sensor
int minZeroReading = 20;  // because the sensor has inherent issues that can cause it to see nothing when there is something

void setup() 
{
  Serial.begin(115200);

  // Initialize the sensor
  if (!sensor_front.begin()) 
  {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1); // Halt if sensor isn't found on A4/A5
  }

//doorServo_right.attach(servoPin_right);
  doorServo_left.attach(servoPin_left);
  doorServo_left.write(0);
}

// pre: 
// post: Open the door by 90 degrees, Mark isOpen to true. Total delay should be 2.54 second, 28.222ms/degree.  
void openDoor() 
{
  if (!isOpen) 
  { 
    for (int pos = 0; pos <= 90; pos++) 
    {
      doorServo_left.write(pos);
      delay(28); 
      delayMicroseconds(222); 
    }
    isOpen = true;
    delay(150);
  }
}

// pre: 
// post: Close the door revert to 0 degree, Mark isOpen to false. Total delay should be 2.54 second, 28.222ms/degree.  
void closeDoor() 
{
  if (isOpen) 
  {
    for (int pos = 90; pos >= 0; pos--) 
    {
      doorServo_left.write(pos);
      delay(28); 
      delayMicroseconds(222); 
    }
    isOpen = false;
    delay(150);
  }
}

// vibe coded, don't question this. it gives the distance accurately. trust me
float getDistance() 
{
  VL53L0X_RangingMeasurementData_t measure;
  
  sensor_front.rangingTest(&measure, false); // "false" means no debug print inside the library

  if (measure.RangeStatus != 4) 
  {  
    // Phase 4 means "Out of range" or "Error"
    return measure.RangeMilliMeter; // Return in mm
  } 
  else 
  {
    return 9999; // Return a huge distance if the sensor is confused
  }
}

void loop() 
{
  float distance = getDistance();

  if (distance > 20000 || distance == 0) 
  { 
    Serial.println("--- SENSOR HANG DETECTED ---");
    
    // Try to reset up to 3 times
    for(int i = 0; i < 3; i++) {
      Serial.print("Reset Attempt "); Serial.println(i+1);
      Wire.begin(); 
      if (sensor_front.begin()) {
        Serial.println("Sensor Online!");
        break; 
      }
      delay(300); // The 300ms settling time you suggested
    }
    return; 
  }

  if(distance != 9999){  Serial.print("Distance (mm): ");   Serial.println(distance);}
  
  if (distance > minDistance && distance <= maxDistance) 
  { 
    zeroReading = 0;  //we reset this because we do see something
    openDoor();           
  } 

  else 
  {
    zeroReading++;  
    
    // we want to do this check because we can ensure we truly see nothing, because the sensor can hallucinate seeing nothing when there's a hand there.
    if (zeroReading >= minZeroReading) 
    {
      closeDoor();
      zeroReading = 0; 
    }
  }

  delay(20);  //gemini says it's good to do this, i suppose. 
}