# JoyStickTest Project
This project takes input from an analog joystick breakout module and converts it to DC motor control board speed values. The joystick is not unlike (or exactly like) an XBOX 360 joystick. 

The code is written for a MicroView OLED module and outputs two dials for the forward and turning speed values, as well as 'flashing' the screen when the button is pressed.

This project is being used to test the feasability of wireless joystick control of the robot.

## Joystick Inputs
X and Y axis analog values from 0 to 1023
Digital momentary switch when the joystick is clicked

## Outputs
X and Y axis values converted (by percentage) to values between -255 and 255, with 0 being no straight (no turning) or stopped.
