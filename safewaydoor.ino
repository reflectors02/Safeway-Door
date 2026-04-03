#include <Servo.h>

Servo doorServo_left;
Servo doorServo_right;

// servos for the door 
const int servoPin_left = 13;
const int servoPin_right = 12;

// front sensor & back sensor
const int trigPin_front = 3;
const int echoPin_front = 2;

const int trigPin_back = 5;
const int echoPin_back = 4;

// misc variables
const int maxDistance = 20; //cm 
const int minDistance = 2; //cm 

bool isOpen = false;     // current state of the door
int zeroReading = 0;     // useful to ensure nothing is truly there infront of the sensor
                         // because the sensor has inherent issues that can cause it to see nothing when there is something

void setup() 
{
  Serial.begin(9600);
  pinMode(trigPin_front, OUTPUT);
  pinMode(echoPin_front, INPUT);
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
  }
}

// vibe coded, don't question this. it gives the distance accurately. trust me
float getDistance() 
{
  digitalWrite(trigPin_front, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin_front, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin_front, LOW);
  long duration = pulseIn(echoPin_front, HIGH, 30000); // 30ms timeout to prevent hanging
  return duration / 58.0;
}

void loop() 
{
  float distance = getDistance();

  if (distance > minDistance && distance <= maxDistance) 
  { 
    zeroReading = 0;  //we reset this because we do see something
    openDoor();           
  } 

  else 
  {
    zeroReading++;  
    
    // we want to do this check because we can ensure we truly see nothing, because the sensor can hallucinate seeing nothing when there's a hand there.
    if (zeroReading >= 30) 
    {
      closeDoor();
      zeroReading = 0; 
    }
  }

  delay(20);  //gemini says it's good to do this, i suppose. 
}