#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>

Servo hServo;
Servo vServo;

byte dataArray[16];

#define SLAVE_ADDRESS 0x08
#define H_SERVO_PIN 5
#define V_SERVO_PIN 3

static int CMD_MOVE_TO = 1;
static int CMD_SET_IDLE = 2;
static int CMD_RESET_POS = 3;
static int CMD_AUTO_IDLE_ON = 4;
static int CMD_AUTO_IDLE_OFF = 5;
static int CMD_STEP = 6;
static int CMD_IDLE_AXIS = 7;
static int CMD_IDLE_SPEED = 8;
static int CMD_STOP = 9;

void receiveEvent(int howMany);
void runIdle();
void showPos();

bool idleMove = false;
bool idleMoveRight = false;
bool idleMoveUp = false;
bool resetPos = false;
bool autoIdle = false;

bool idleV = true;
bool idleH = true;


volatile int idleSpeed = 300;
volatile int idleVSpeed;
volatile int idleHSpeed;

const int hMax = 180;
const int hMin = 0;

const int vMax = 115;
const int vMin = 70;

int startingPosH = 90;
int startingPosV = 90;

volatile int newPosH;
volatile int newPosV;

volatile unsigned long lastAction;
const unsigned long triggerIdleAfter = 60000; // 6s

void setup() {
  newPosH = startingPosH;
  newPosV = startingPosV;
  lastAction = millis();

  hServo.attach(H_SERVO_PIN);
  vServo.attach(V_SERVO_PIN);
  delay(1000);

  hServo.write(newPosH);
  vServo.write(newPosV);

  delay(1000);
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(showPos);
}

void showPos() {
  Wire.write(newPosV);
  Wire.write(newPosH);
}

void receiveEvent(int howMany)
{
  int cmd = Wire.read();
  int arg1 = Wire.read();
  int arg2 = Wire.read();

  int position;

  if (cmd == 0)
    return;

  lastAction = millis();
  Servo *servo;
  
  if (cmd == CMD_STEP || cmd == CMD_MOVE_TO) {
    if (arg1 == 1) {
    servo = &hServo;
  
    if (cmd == CMD_STEP){
      if (position == 1) {
        newPosH++;
      } else if (position == 0) {
        --newPosH;
      }
      position = newPosH;
    }
    if (position > hMax) {
      position = hMax;
    }
    if (position < hMin) {
      position = hMin;
    }
    newPosH = position;
  } else {
    servo = &vServo;

    if (cmd == CMD_STEP) {
      if (position == 1) {
        newPosV++;
      }
      else if (position == 0) {
        --newPosV;
      }
      position = newPosV;
    }
    if (position > vMax) {
      position = vMax;
    }
    if (position < vMin) {
      position = vMin;
    }
    newPosV = position;
  }
    int position = arg2;

    if (arg1 == 1) {
      servo = &hServo;

      if (cmd == CMD_STEP) {
        if (position == 1) {
          newPosH++;
        } else if (position == 0) {
          --newPosH;
        }
        position = newPosH;
      }
      if (position > hMax) {
        position = hMax;
      }
      if (position < hMin) {
        position = hMin;
      }
      newPosH = position;
    } else {
      servo = &vServo;

      if (cmd == CMD_STEP) {
        if (position == 1) {
          newPosV++;
        } else if (position == 0) {
          --newPosV;
        }
        position = newPosV;
      }
      if (position > vMax) {
        position = vMax;
      } else if (position < vMin) {
        position = vMin;
      }
      newPosV = position;
    }
  }


  if (cmd == CMD_MOVE_TO || cmd == CMD_STEP) {
    idleMove = false;
    (*servo).write(position);
  } else if (cmd == CMD_SET_IDLE) {
    idleMove = true;
  } else if (cmd == CMD_RESET_POS) {
    idleMove = false;
    resetPos = true;
  } else if (cmd == CMD_AUTO_IDLE_ON) {
    autoIdle = true;
  } else if (cmd == CMD_AUTO_IDLE_OFF) {
    autoIdle = false;
  } else if (cmd == CMD_IDLE_AXIS) {
    bool *axisIdle;
    if (arg1 == 1) {
      axisIdle = &idleH;
    } else {
      axisIdle = &idleV;
    }

    if (arg2 == 1) {
      *axisIdle = true;
    } else {
      *axisIdle = false;
    }
  } else if (cmd == CMD_IDLE_SPEED) {
    idleSpeed = 10 * arg1 + arg2;
  } else if (cmd == CMD_STOP) {
    idleMove = false;
  }
}

void runIdle()
{
  if (idleH) {
    if (idleMoveRight)
    {
      if (newPosH <= hMax)
      {
        hServo.write(newPosH);
        newPosH++;
      }
      else
      {
        newPosH = hMax;
        idleMoveRight = false;
      }
    }
    else
    {
      if (newPosH >= hMin)
      {
        hServo.write(newPosH);
        --newPosH;
      }
      else
      {
        newPosH = hMin;
        idleMoveRight = true;
      }
    }
  }
  
  if (idleV) {
    if (idleMoveUp)
    {
      if (newPosV <= vMax)
      {
        vServo.write(newPosV);
        newPosV++;
      }
      else
      {
        newPosV = vMax;
        idleMoveUp = false;
      }
    }
    else
    {
      if (newPosV >= vMin)
      {
        vServo.write(newPosV);
        --newPosV;
      }
      else
      {
        newPosV = vMin;
        idleMoveUp = true;
      }
    }
  }
  
  delay(idleSpeed);
}

void reset() {
  newPosH = hMin;
  newPosV = vMin;

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