#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>

// PINS
#define H_SERVO_PIN 5
#define V_SERVO_PIN 3

#define SLAVE_ADDRESS 0x08

// COMMANDS
#define CMD_MOVE_TO 1
#define CMD_SET_IDLE 2
#define CMD_RESET_POS 3
#define CMD_AUTO_IDLE_ON 4
#define CMD_AUTO_IDLE_OFF 5
#define CMD_STEP 6
#define CMD_IDLE_AXIS 7
#define CMD_IDLE_SPEED 8
#define CMD_STOP 9

// MOTOR SETTINGS
#define DEFAULT_IDLE_SPEED 300

#define H_STARTING_POS 90
#define V_STARTING_POS 90

#define H_MIN 0
#define H_MAX 180
#define V_MIN 75
#define V_MAX 115

Servo hServo;
Servo vServo;

byte dataArray[16];

void receiveEvent(int howMany);
void runIdle();
void showStatus();
void stepHandler(unsigned int position, unsigned int motor);
void moveHandler(unsigned int position, unsigned int motor);
void attach();
void detach();

bool idleMove = false;
bool idleMoveRight = true;
bool idleMoveUp = true;
bool resetPos = false;
bool autoIdle = false;

bool idleV = true;
bool idleH = true;

bool cmdReceived = false;
int cmd = 0;
int arg1 = 0;
int arg2 = 0;

volatile int idleSpeed = DEFAULT_IDLE_SPEED;
volatile int idleVSpeed;
volatile int idleHSpeed;

int startingPosH = H_STARTING_POS;
int startingPosV = V_STARTING_POS;

volatile int newPosH;
volatile int newPosV;

volatile unsigned long lastAction;
const unsigned long triggerIdleAfter = 6000; // 6s
const unsigned long offAfter = 8000; // 8s

void setup() {
  newPosH = startingPosH;
  newPosV = startingPosV;
  lastAction = millis();

  attach();
  
  hServo.write(newPosH);
  vServo.write(newPosV);
  delay(1000);
  detach();

  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(showStatus);
}

void attach() {
  if (!hServo.attached()) {
    hServo.attach(H_SERVO_PIN);
  }
  if (!vServo.attached()) {
    vServo.attach(V_SERVO_PIN);
  }
  delay(200);
}

void detach() {
  if (hServo.attached())
  {
    hServo.detach();
  }
  if (vServo.attached())
  {
    vServo.detach();
  }
}

void showStatus() {
  Wire.write(newPosH);
  Wire.write(newPosV);
  Wire.write(idleMove);
  Wire.write(idleH);
  Wire.write(idleV);
  Wire.write(autoIdle);
  Wire.write(idleSpeed);
}

void receiveEvent(int howMany)
{
  cmd = Wire.read();
  arg1 = Wire.read();
  arg2 = Wire.read();

  if (cmd == 0)
    return;

  lastAction = millis();
  cmdReceived = true;
}

void handleCommand() {
  if (cmd == CMD_STEP)
  {
    idleMove = false;
    stepHandler(arg2, arg1);
  }
  else if (cmd == CMD_MOVE_TO)
  {
    idleMove = false;
    moveHandler(arg2, arg1);
  }
  else if (cmd == CMD_SET_IDLE)
  {
    attach();
    idleMove = true;
  }
  else if (cmd == CMD_RESET_POS)
  {
    attach();
    idleMove = false;
    resetPos = true;
    idleH = true;
    idleV = true;
    idleSpeed = DEFAULT_IDLE_SPEED;
  }
  else if (cmd == CMD_AUTO_IDLE_ON)
  {
    attach();
    autoIdle = true;
  }
  else if (cmd == CMD_AUTO_IDLE_OFF)
  {
    autoIdle = false;
  }
  else if (cmd == CMD_IDLE_AXIS)
  {
    bool *axisIdle;
    if (arg1 == 1)
    {
      axisIdle = &idleH;
    }
    else
    {
      axisIdle = &idleV;
    }

    if (arg2 == 1)
    {
      *axisIdle = true;
    }
    else
    {
      *axisIdle = false;
    }
  }
  else if (cmd == CMD_IDLE_SPEED)
  {
    idleSpeed = 10 * arg1 + arg2;
  }
  else if (cmd == CMD_STOP)
  {
    idleMove = false;
  }
}

Servo* getServoById(unsigned int servoId) {
  if (servoId == 1) {
    return &hServo;
  }

  return &vServo;
}

void runIdle()
{
  if (idleH) {
    if (idleMoveRight) {
      if (newPosH < H_MAX) {
        hServo.write(newPosH);
        newPosH++;
      }
      else {
        newPosH = H_MAX;
        idleMoveRight = false;
      }
    }
    else {
      if (newPosH > H_MIN) {
        hServo.write(newPosH);
        newPosH--;
      }
      else {
        newPosH = H_MIN;
        idleMoveRight = true;
      }
    }
  }
  
  if (idleV) {
    if (idleMoveUp) {
      if (newPosV < V_MAX) {
        vServo.write(newPosV);
        newPosV++;
      }
      else {
        newPosV = V_MAX;
        idleMoveUp = false;
      }
    }
    else {
      if (newPosV > V_MIN) {
        vServo.write(newPosV);
        newPosV--;
      }
      else {
        newPosV = V_MIN;
        idleMoveUp = true;
      }
    }
  }
  
  delay(idleSpeed);
}

int adjustPosToLimits(unsigned int position, int servo) {

  if (servo == 1) {
    if (position > H_MAX) {
      position = H_MAX;
    }
    if (position < H_MIN) {
      position = H_MIN;
    }
  } else {
    if (position > V_MAX) {
      position = V_MAX;
    }
    if (position < V_MIN) {
      position = V_MIN;
    }
  }

  return position;
}

void stepHandler(unsigned int position, unsigned int motor) {
  if (motor == 1) {
    if (position == 1) {
      newPosH++;
    }
    else if (position == 0) {
      --newPosH;
    }
    newPosH = position = adjustPosToLimits(newPosH, motor);
  } else {
    if (position == 1) {
      newPosV++;
    }
    else if (position == 0) {
      --newPosV;
    }
    newPosV = position = adjustPosToLimits(newPosV, motor);
  }
  attach();

  (getServoById(motor))->write(position);
}

void moveHandler(unsigned int position, unsigned int motor) {
  position = adjustPosToLimits(position, motor);
  
  if (motor == 1) {
    newPosH = position;
  } else {
    newPosV = position;
  }
  attach();

  (getServoById(motor))->write(position);
}

void reset() {
  newPosH = startingPosH;
  newPosV = startingPosV;

  hServo.write(newPosH);
  delay(1000);
  vServo.write(newPosV);
  delay(1000);

  idleMoveRight = true;
  idleMoveUp = true;
  resetPos = false;
}

void loop() {
  unsigned long offset = millis() - lastAction;

  if (cmdReceived)
  {
    cmdReceived = false;
    handleCommand();
  }

  if (autoIdle && (offset > triggerIdleAfter)) {
    idleMove = true;
  }

  if (!idleMove && offset > offAfter) {
    detach();
  }

  if (resetPos) {
    reset();
  }

  if (idleMove) {
    runIdle();
  }
}