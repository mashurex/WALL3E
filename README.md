# WALL3E
Arduino robot firmware projects and exploration. 
The main project codebase are the WALL3E_x Arduino projects and they are an extremely basic codebase for obstacle detection using three sonar range finders and remote control via analog joystick and NRF24L01 transceivers.

## Arduino Mega Bot Controller
- (1) Three wire PING))) sonar module
- (2) Four wire HC-SR04 sonar modules (on the left and right)
- (1) OSEPP Motor Control Board
- (4) DC motors
- (1) 6mAh LiPoly pack for powering motors via the motor control board
- (1) LiPo boost converter for 6mAh LiPoly pack
- (1) 4AA battery pack for powering the Arduino Mega
- connected via I2C to an Arduino Uno radio receiver

## Arduino Uno Bot Receiver
- (1) NRF24L01+ for receiving commands from the remote control
- connected via I2C to the Arduino Mega bot controller

## Arduino Uno Remote Controller
- (1) NRF24L01+ for transmitting coded joystick/button commands to the receiver
- (1) Analog joystick with push button
- (1) SparkFun LiPower shield for LiPoly power supply and recharging

## Why WALL3E?
My wife said it looked kind of like WALL-E but with three pairs of eyes. Also, why not?

# Scratch Directory
The Scratch Directory contains small snippets of code used to test various parts of development. Code from projects in there may or may not be in the main project code, but it applied to something that was being explored, such as NRF2401 wireless communication, Joystick remote control, etc.


