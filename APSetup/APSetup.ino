#include <ESP8266WiFi.h>
#include <MobaTools.h>
#include <EEPROM.h>

MoToServo servo;
MoToStepper stepper(200);

const char *ssid = "ESP8266-AP";
const unsigned int servoPin = D4;
const unsigned int dirPin = D5;
const unsigned int stepPin = D6;
const unsigned int enPin = D7;
unsigned int servoSpeed = 5;
unsigned int stepperSpeed = 5;
unsigned int speedChange = 1;

WiFiServer server(80);

void setup() 
{
  Serial.begin(9600);
  WiFi.softAP(ssid);
  server.begin();
  pinMode(enPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(servoPin, OUTPUT);
  servo.attach(servoPin);
  servo.setSpeed(servoSpeed);
  stepper.setSpeed(stepperSpeed);
}

int isRotationCCW (String rotation)
{
  return rotation != "CCW" ? 1 : -1;
}

void adjustServoPosition(int adjustment)
{
  int pos = servo.read();
  if (pos >= 0 && pos <= 90)
  {
    servo.write(pos + adjustment);
  }
}

void adjustStepperPosition(int adjustment, unsigned int steps)
{
  stepper.move(steps * adjustment); 
}

bool isMotorServo(String motor)
{
  return motor == "servo";
}

void adjustPositionMotor(String direction, unsigned int steps, bool isServo)
{
  int adjustment = isRotationCCW(direction);

  if (isServo)
  {
    adjustServoPosition(adjustment);
  }
  else 
  {
    adjustStepperPosition(adjustment, steps);
  }
}

void adjustSpeedMotor(String motor, String direction)
{
  bool isDecreasingSpeed = isRotationCCW(direction) == -1;
  unsigned int* speedPtr = isMotorServo(motor) ? &servoSpeed : &stepperSpeed;
  bool canDecreaseSpeed = *speedPtr > 0 && isDecreasingSpeed; 
  if (canDecreaseSpeed || !isDecreasingSpeed)
  {
    *speedPtr += isDecreasingSpeed ? -speedChange : speedChange;
  }
  if (isMotorServo(motor))
  {
    servo.setSpeed(servoSpeed);
  }
  else 
  {
    stepper.setSpeedSteps(stepperSpeed);
  }
}

void loop() 
{
  WiFiClient client = server.available();
  if (client)
  {
    if (!client.available())
    {
      delay(1);
    }
    else
    {
      String request = client.readStringUntil('\n');
      unsigned int commaIndex = request.indexOf(',');
      unsigned int spaceIndex = request.indexOf(' ');
      String motorType = request.substring(0, commaIndex);
      String rotation = request.substring(spaceIndex + 1, request.length() -1);
      if (motorType == "speed")
      {
        adjustSpeedMotor(motorType, rotation);
      }
      else
      {
        adjustPositionMotor(rotation, 1, isMotorServo(motorType));
      }
      client.flush();
    }
  }
}
