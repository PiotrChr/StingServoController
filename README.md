# Sting Camera Robot Controller

This README details the functionalities and usage of the Sting Camera Robot Controller, a system built on the Arduino platform to control a camera's position with high precision using servo motors. It is designed for applications requiring remote and automated camera movements, such as surveillance, filming, or photography.

## Features

- **Dual Servo Control**: Independently controls horizontal and vertical servo motors attached to the camera.
- **Precise Positioning**: Moves the camera to specific angles using the horizontal (H) and vertical (V) servos.
- **Idle Movements**: Automatically idles the camera in a sweeping motion to cover a broader area when not actively controlled.
- **Customizable Limits**: Configures minimum and maximum servo angles to prevent hardware damage or operational issues.
- **Remote Commands**: Receives commands via the I2C interface to adjust positions, set idle behavior, and more.

## Hardware Requirements

- Arduino board (Uno, Mega, etc.)
- Two servo motors (for horizontal and vertical movement)
- Suitable power supply for servos
- Connections for I2C communication (SCL, SDA pins)

## Setup and Installation

1. **Connect Servos**: Attach the horizontal servo to pin 5 and the vertical servo to pin 3 of your Arduino.
2. **I2C Setup**: Ensure that the Arduino is set up as an I2C slave device using the specified slave address (0x08).

## Usage

The controller listens for specific commands over the I2C interface to perform actions such as moving the servos to desired positions or toggling idle movements. Commands are as follows:

- **CMD_MOVE_TO (1)**: Move to a specified angle. Requires additional arguments for axis (1 for horizontal, 2 for vertical) and angle.
- **CMD_SET_IDLE (2)**: Toggle idle movement on or off.
- **CMD_RESET_POS (3)**: Reset servos to their starting positions.
- **CMD_AUTO_IDLE_ON (4)**: Automatically enable idle movements after a period of inactivity.
- **CMD_AUTO_IDLE_OFF (5)**: Disable automatic idle movements.
- **CMD_STEP (6)**: Make a small adjustment to the servo position. Useful for fine-tuning.
- **CMD_IDLE_AXIS (7)**: Set specific axis to idle.
- **CMD_IDLE_SPEED (8)**: Adjust the speed of idle movements.
- **CMD_STOP (9)**: Immediately stop all movements.

## Functions

- **attach()**: Activates the servo motors for movement.
- **detach()**: Deactivates the servo motors to save power.
- **showStatus()**: Sends current status information via I2C.
- **receiveEvent()**: Handles incoming I2C commands.
- **runIdle()**: Controls the idle sweeping motion of the servos based on set parameters.

## Customization

Adjust the `H_MIN`, `H_MAX`, `V_MIN`, and `V_MAX` defines in the code to set the movement boundaries specific to your camera setup and mounting configuration to ensure safe operation.

## Safety and Precautions

Ensure all mechanical parts are securely fastened and that the servo limits are set to prevent the camera from hitting any part of its enclosure or mount.

## License

This project is provided "as is", without warranty of any kind. Use at your own risk.
