/*******************************************************************
  Developer -- Mandy Wells
  Course ----- EECE 3624
  Project ---- ZumoBot Final Project

  Controls the Zumo robot using Bluetooth commands from the ReadyAVR.
  The reflectance sensors override movement commands when a boundary
  is detected.
*******************************************************************/
#include <Wire.h>
#include <ZumoShield.h>
#include <SoftwareSerial.h>

// Boundary detection
#define QTR_THRESHOLD  1500

// Motor speeds
#define REVERSE_SPEED     200
#define TURN_SPEED        200
#define FORWARD_SPEED     200

// Motor movement durations
#define REVERSE_DURATION  200
#define TURN_DURATION     300

// Number of reflectance sensors
#define NUM_SENSORS 6

// HC-05 Bluetooth connection
// Arduino pin 2 = RX
// Arduino pin 4 = TX (not physically connected)
SoftwareSerial bluetooth(2, 4);

// Zumo components
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);
ZumoReflectanceSensorArray sensors(QTR_NO_EMITTER_PIN);

unsigned int sensor_values[NUM_SENSORS];
unsigned char incomingByte;

int speedMode = 0;
int currentSpeed = 200;

// Stores the current movement command
char currentCommand = 'S';

// Read a command from Bluetooth
void readBluetooth(){
  if(bluetooth.available()){
    incomingByte = (unsigned char) bluetooth.read();

    switch(incomingByte){
      case 'F':
        currentCommand = 'F';
        break;

      case 'B':
        currentCommand = 'B';
        break;

      case 'L':
        currentCommand = 'L';
        break;

      case 'R':
        currentCommand = 'R';
        break;

      case 'S':
        currentCommand = 'S';
        break;

      case 'V':
        speedMode++;

        if(speedMode > 2){
          speedMode = 0;
        }

        if(speedMode == 0){
          currentSpeed = 200;
          Serial.println("Slow Speed");
        }
        else if(speedMode == 1){
          currentSpeed = 275;
          Serial.println("Medium Speed");
        }
        else{
          currentSpeed = 350;
          Serial.println("Fast Speed");
        }
        break;

      default: break;
    }

    // Display the received command for debugging
    Serial.write(incomingByte);
    Serial.println();
  }
}


// Reverse and turn away from the detected boundary
void escapeBoundary(bool leftEdge){
  // Reverse away from the boundary
  motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
  delay(REVERSE_DURATION);

  if (leftEdge){
    // Left sensor detects boundary, turn right
    motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
  }
  else{
    // Right sensor detects boundary, turn left
    motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
  }

  delay(TURN_DURATION);

  // Stop after escaping the boundary
  currentCommand = 'S';
  motors.setSpeeds(0, 0);
}


// Check the reflectance sensors for a boundary
bool checkBoundary(){
  sensors.read(sensor_values);

  // Leftmost sensor detects boundary
  if(sensor_values[0] > QTR_THRESHOLD){
    escapeBoundary(true);
    return true;
  }
  // Rightmost sensor detects boundary
  else if(sensor_values[5] > QTR_THRESHOLD){
    escapeBoundary(false);
    return true;
  }

  return false;
}


// Perform the current movement command
void moveRobot(){
  switch(currentCommand){
    case 'F':
      motors.setSpeeds(currentSpeed, currentSpeed);
      break;

    case 'B':
      motors.setSpeeds(-currentSpeed, -currentSpeed);
      break;

    case 'L':
      motors.setSpeeds(-currentSpeed, currentSpeed);
      break;

    case 'R':
      motors.setSpeeds(currentSpeed, -currentSpeed);
      break;

    case 'S':
      motors.setSpeeds(0, 0);
      break;
  }
}


void setup(){
  pinMode(LED, OUTPUT);
  Serial.begin(9600);

  // Start the HC-05 Bluetooth connection
  bluetooth.begin(9600);

  // Keep the robot stopped until it receives a command
  motors.setSpeeds(0, 0);

  Serial.println("ZumoBot ready.");
}


void loop(){
  // Check for new Bluetooth commands
  readBluetooth();

  // Boundary detection takes priority over movement
  if(!checkBoundary()){
    moveRobot();
  }
}
