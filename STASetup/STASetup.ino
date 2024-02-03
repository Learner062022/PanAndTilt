#include <ESP8266WiFi.h>
#include <Bounce2.h>
#include <MobaTools.h>

const char *ssid = "ESP8266-AP";
const unsigned int clkPinThermal = D4;
const unsigned int dtPinThermal = D3;
const unsigned int swPinThermal = D2;

const unsigned int clkPinSpeed = D8;
const unsigned int dtPinSpeed = D7;
const unsigned int swPinSpeed = D6;

unsigned long bounceDuration = 5;
Bounce debouncerClkPinThermal = Bounce();
Bounce debouncerSwPinThermal = Bounce();
Bounce debouncerClkPinSpeed = Bounce();
WiFiClient client = WiFiClient();
bool encoderClicked = false;

void setup() 
{
  Serial.begin(9600);

  pinMode(clkPinThermal, INPUT_PULLUP);
  pinMode(dtPinThermal, INPUT_PULLUP);
  pinMode(swPinThermal, INPUT_PULLUP);

  pinMode(clkPinSpeed, INPUT_PULLUP);
  pinMode(dtPinSpeed, INPUT_PULLUP);

  debouncerClkPinThermal.attach(clkPinThermal);
  debouncerSwPinThermal.attach(swPinThermal);
  debouncerClkPinSpeed.attach(clkPinSpeed);

  debouncerClkPinThermal.interval(bounceDuration);
  debouncerSwPinThermal.interval(bounceDuration);
  debouncerClkPinSpeed.interval(bounceDuration);
  
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print('.');
  }
  client.connect("192.168.4.1", 80); 
}

bool isInstanceEqual(Bounce& inst, Bounce& target)
{
  return &inst == &target;
}

void processEncoderEvent(int dtPin, const String& messagePrefix)
{
  String direction = digitalRead(dtPin) == HIGH ? "CLK" : "CCW";
  String message = messagePrefix + direction;
  client.println(message);
}

void processDebounceEvent(Bounce& inst, int dtPin, const String& messagePrefix)
{
  inst.update();
  if (inst.fell())
  {
    if (isInstanceEqual(inst, debouncerSwPinThermal))
    {
      encoderClicked = !encoderClicked;
    }
    else if (isInstanceEqual(inst, debouncerClkPinThermal))
    {
      String prefix = encoderClicked ? "servo, " : "stepper, ";
      processEncoderEvent(dtPin, prefix);
    }
    else if (isInstanceEqual(inst, debouncerClkPinSpeed))
    {
      processEncoderEvent(dtPin, "speed, ");
    }
  }
}

void loop() 
{
  processDebounceEvent(debouncerSwPinThermal, dtPinThermal, "");
  processDebounceEvent(debouncerClkPinThermal, dtPinThermal, "");
  processDebounceEvent(debouncerClkPinSpeed, dtPinSpeed, "");
}
