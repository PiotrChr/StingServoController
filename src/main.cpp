#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>

Servo hServo;
Servo vServo;

byte dataArray[16];

#define SLAVE_ADDRESS 0x08
#define H_SERVO_PIN 5
#define V_SERVO_PIN 3

int MOVE_TO_CMD = 1;
int SET_IDLE = 2;
int RESET_POS = 3;
int AUTO_IDLE_ON = 4;
int AUTO_IDLE_OFF = 5;

void receiveEvent(int howMany);
void moveTo(Servo &servo, int position);
void runIdle();

bool idleMove = false;
bool idleMoveRight = false;
bool idleMoveUp = false;
bool resetPos = false;
bool autoIdle = false;

int hMax = 180;
int hMin = 0;

int vMax = 115;
int vMin = 70;

int startingPosH = 0;
int startingPosV = 70;

int newPosH = startingPosH;
int newPosV = startingPosV;

unsigned long lastAction;
unsigned long triggerIdleAfter = 120;

void setup() {
  lastAction = millis();
  Serial.begin(9600);

  hServo.attach(H_SERVO_PIN);
  vServo.attach(V_SERVO_PIN);

  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
}

void receiveEvent(int howMany)
{
  lastAction = millis();
  int cmd = Wire.read();
  int servoNo = Wire.read();
  int position = Wire.read();
  
  Servo *servo;

  if (servoNo == 1) {
    servo = &hServo;

    if (position > hMax) {
      position = hMax;
    }
    if (position < hMin)
    {
      position = hMin;
    }
  } else {
    servo = &vServo;
    if (position > vMax)
    {
      position = vMax;
    }
    if (position < vMin)
    {
      position = vMin;
    }
  }

  if (cmd == MOVE_TO_CMD) {
    idleMove = false;
    (*servo).write(position);
  }
  
  if (cmd == SET_IDLE)
  {
    resetPos = true;
    idleMove = true;
  }

  if (cmd == RESET_POS)
  {
    idleMove = false;
    resetPos = true;
  }

  if (cmd == AUTO_IDLE_ON)
  {
    autoIdle = true;
  }

  if (cmd == AUTO_IDLE_OFF)
  {
    autoIdle = false;
  }
}

void moveTo(Servo &servo, int position) {
  servo.write(position);
}

void runIdle()
{
  if (idleMoveRight) {
    if (newPosH <= hMax) {
      hServo.write(newPosH);
      newPosH++;
    } else {
      newPosH = hMax;
      idleMoveRight = false;
    }
  } else {
    if (newPosH >= hMin) {
      hServo.write(newPosH);
      --newPosH;
    } else {
      newPosH = hMin;
      idleMoveRight = true;
    }
  }

  if (idleMoveUp) {
    if (newPosV <= vMax) {
      vServo.write(newPosV);
      newPosV++;
    } else {
      newPosV = vMax;
      idleMoveUp = false;
    }
  } else {
    if (newPosV >= vMin) {
      vServo.write(newPosV);
      --newPosV;
    } else {
      newPosV = vMin;
      idleMoveUp = true;
    }
  }

  delay(300);
}

void reset() {
  hServo.write(hMin);
  delay(1000);
  vServo.write(vMin);
  delay(1000);

  idleMoveRight = true;
  idleMoveUp = true;
  resetPos = false;
}

void loop() {
  if (autoIdle && (millis() - lastAction > triggerIdleAfter)) {
    idleMove = true;
  }

  if (resetPos) {
    reset();
  }

  if (idleMove) {
    runIdle();
  }
  
}