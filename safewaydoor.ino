#include <Servo.h>
#include "Adafruit_VL53L0X.h"

Servo doorServo_left;
Servo doorServo_right;

Adafruit_VL53L0X sensor_front = Adafruit_VL53L0X();
Adafruit_VL53L0X sensor_back = Adafruit_VL53L0X();

// servos for the door 
const int servoPin_left = 13;
const int servoPin_right = 12;

// sensors for the door
const int SHT_FRONT = 2;
const int SHT_BACK = 3;

// misc variables
const int maxDistance = 100; // The VL53L0X works in mm, so 200mm = 20cm
const int minDistance = 0;  // 20mm = 2cm

bool isOpen = false;      // current state of the door
int zeroReading = 0;      // useful to ensure nothing is truly there infront of the sensor
int minZeroReading = 20;  // because the sensor has inherent issues that can cause it to see nothing when there is something

void setup() 
{
  Serial.begin(115200);

  pinMode(SHT_FRONT, OUTPUT);
  pinMode(SHT_BACK, OUTPUT);

  // STEP 1: Turn off the sensors, we do this because we want to turn them on one by one.
  digitalWrite(SHT_FRONT, LOW);
  digitalWrite(SHT_BACK, LOW);
  delay(10);

  // STEP 2: We turn on front sensor first
  digitalWrite(SHT_FRONT, HIGH);
  delay(10); 
  if (!sensor_front.begin(0x30)) 
  {
    Serial.println(F("Failed to boot VL53L0X_front"));
    while (1); // Halt if sensor isn't found on A4/A5
  }

  // STEP 3: Turn on the back sensor
  digitalWrite(SHT_BACK, HIGH);
  delay(10);
  if(!sensor_back.begin(0x31))
  {
    Serial.println(F("Failed to boot VL53L0X_back"));
    while (1); // Halt if sensor isn't found on A4/A5
  }

  Serial.println(F("Both Sensors Online: Front (0x30), Back (0x31)"));

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
      //doorServo_right.write(-pos);
      delay(28); 
      delayMicroseconds(222); 
    }
    isOpen = true;
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
      //doorServo_right.write(-pos);
      delay(28); 
      delayMicroseconds(222); 
    }
    isOpen = false;
  }
}

// pre: A sensor, either front or back is passed in.
// post: returns distance, print the distance out in mm in the serial. 
//       If no reading or sensor is confused, return 9999mm.
//       If front sensor it should say: Front Sensor: + distance
//       If back sensor it should say: Back Sensor: + distance
float getDistance(Adafruit_VL53L0X &sensor) 
{
  VL53L0X_RangingMeasurementData_t measure;
  
  sensor.rangingTest(&measure, false); 

  if (measure.RangeStatus != 4) 
  {  
    if (&sensor == &sensor_front && measure.RangeMilliMeter <= maxDistance) 
    {
      Serial.print("Front Sensor: ");
      Serial.print(measure.RangeMilliMeter);
      Serial.println("mm");

    } 
    else if(measure.RangeMilliMeter <= maxDistance)
    {
      Serial.print("Back Sensor: ");
      Serial.print(measure.RangeMilliMeter);
      Serial.println("mm");
    }

    return measure.RangeMilliMeter; 
  } 
  else 
  {
    return 9999; 
  }
}

// pre: If a sensor is disconnected, maybe due to sudden voltage drop. We need to reset it
// post: Reset the sensor, reconnect it
// NOTE: THIS IS VIBE CODED, don't question it
void fixSensorHang(Adafruit_VL53L0X &sensor) 
{
  int targetAddress = (&sensor == &sensor_front) ? 0x30 : 0x31;
  int shutPin = (&sensor == &sensor_front) ? SHT_FRONT : SHT_BACK;

  // PHYSICS RESET: Toggle the power via XSHUT
  digitalWrite(shutPin, LOW);
  delay(100);
  digitalWrite(shutPin, HIGH);
  delay(100);

  for(int i = 0; i < 3; i++) 
  {
    if (sensor.begin(targetAddress)) { // Return to the correct identity
      Serial.println("Sensor Re-synchronized!");
      return; 
    }
    delay(100); 
  }
}


void loop() 
{
  float distance_front = getDistance(sensor_front);
  float distance_back = getDistance(sensor_back);

  if(distance_front > 20000){fixSensorHang(sensor_front);}
  if(distance_back > 20000){fixSensorHang(sensor_back);}

  if ((distance_front > minDistance && distance_front <= maxDistance) || (distance_back > minDistance && distance_back <= maxDistance)) 
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







// DEBUG TOOLS, DO NOT TOUCH
  // if (distance > 20000 || distance == 0) 
  // { 
  //   Serial.println("--- SENSOR HANG DETECTED ---");
    
  //   // Try to reset up to 3 times
  //   for(int i = 0; i < 3; i++) {
  //     Serial.print("Reset Attempt "); Serial.println(i+1);
  //     Wire.begin(); 
  //     if (sensor_front.begin()) {
  //       Serial.println("Sensor Online!");
  //       break; 
  //     }
  //     delay(300); 
  //   }
  //   return; 
  // }