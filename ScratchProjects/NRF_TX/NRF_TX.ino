
/**
 * Sample sketch for taking joystick input, encoding it, and transmitting over
 * radio via NRF24L01+.
 */

#include <SPI.h>
#include <RF24.h>
#include <stdlib.h>

const uint8_t PIN_VERT = A1;
const uint8_t PIN_HORIZ = A2;
const uint8_t PIN_PUSH = 6;
const uint8_t PIN_CE = 7;
const uint8_t PIN_CSN = 8;

// General resting (centered) values for the joystick.
const int REST_Y = 506;
const int REST_X = 518;

const int MAX_JOYSTICK = 1023;
const int MIN_JOYSTICK = 0;

// +/- Input value floating range.
const int FLT_JOYSTICK = 5;

const uint8_t MIN_SPEED = 0;
const uint8_t MAX_SPEED = 255;
const uint8_t DIR_FORWARD = 1;
const uint8_t DIR_BACKWARD = 2;
const uint8_t DIR_LEFT = 3;
const uint8_t DIR_RIGHT = 4;
const uint8_t DIR_NONE = 0;

byte addresses[][6] = {"1Node","2Node"};

RF24 radio(PIN_CE, PIN_CSN);

int X = 0;
int Y = 0;

void setup() 
{
  Serial.begin(115200);
  
  pinMode(PIN_PUSH, INPUT);  
  digitalWrite(PIN_PUSH,HIGH);
  Serial.println(F("JoyStick push button initialized..."));
  
  radio.begin();
  radio.openWritingPipe(addresses[1]); 
  Serial.println(F("Radio transmission line open..."));
}

void loop() 
{  
  X = analogRead(PIN_HORIZ);
  Y = analogRead(PIN_VERT);
  int pressed = digitalRead(PIN_PUSH);

  X = getSpeedFromAnalog(X, REST_X);
  Y = getSpeedFromAnalog(Y, REST_Y);

  char output[10];
  sprintf(output, "%s%03d%s%03d%s", 
    // X +/- and absolute speed value in three digits
    (X >= 0) ? "+" : "-", abs(X),
    // Y +/- and absolute speed value in three digits
    (Y >= 0) ? "+" : "-", abs(Y),
    // Send an X if the button is pressed, O for open
    (pressed == 0) ? "X" : "O");
    
  Serial.println(output);
  radio.write( &output, sizeof(output) );  
  
  delay(250);
}

/**
 * Negative values are backward or left, depending on the axis being measured.
 */
int getSpeedFromAnalog(int input, int restPoint)
{  
  if(input >= MIN_JOYSTICK && input <= FLT_JOYSTICK){ return 255; }
  else if(input <= MAX_JOYSTICK && input >= (MAX_JOYSTICK - FLT_JOYSTICK)){ return -255; }
  else if((input >= (restPoint - FLT_JOYSTICK)) && (input <= (restPoint + FLT_JOYSTICK)))
  { 
    return 0; 
  }  
  else 
  {
    float range = (float)restPoint;
    float speedValue = 0;
    if(input > restPoint)
    {
      // The higher the speed the faster it is going in reverse.
      range = (float)(MAX_JOYSTICK - restPoint);
      // Multiply by -1 to flip the speed value to indicate reverse.
      speedValue = (((input - restPoint) / range) * MAX_SPEED) * -1;      
    }
    else
    {
      // We need to flip the percentage as this works backwards, with 0 being the fastest.
      speedValue = (1 - (input / range)) * 255;      
    }
    
    // Truncate to an integer to return a speed value between -255 and 255.
    return (int)speedValue;
  }
}

