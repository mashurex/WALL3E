
/**
 * Sketch for taking joystick input, encoding it, and transmitting over
 * radio via NRF24L01+ using an Arduino Uno.
 * 
 * Created by Mustafa Ashurex <ashurexm@gmail.com> 
 */

#include <SPI.h>
#include <RF24.h>
#include <stdlib.h>
#include <Wire.h>

// Address for LiPower shield on I2C bus.
#define MAX17043_ADDRESS 0x36

// Controller shield pins.
#define PIN_VERT = A1;
#define PIN_HORIZ = A0;
#define PIN_PUSH = 2;
#define PIN_BTN_RIGHT = 3;
#define PIN_BTN_UP = 4;
#define PIN_BTN_DOWN = 5;
#define PIN_BTN_LEFT = 6;

// Radio pins.
#define PIN_CE = 7;
#define PIN_CSN = 8;

// General resting (centered) values for the joystick.
#define REST_Y = 515;
#define REST_X = 518;

// Min/max analog value ranges for the joystick axes.
#define MAX_JOYSTICK = 1023;
#define MIN_JOYSTICK = 0;

// +/- Input value floating range.
#define FLT_JOYSTICK = 5;

#define MIN_SPEED = 0;
#define MAX_SPEED = 255;

#define DIR_FORWARD = 1;
#define DIR_BACKWARD = 2;
#define DIR_LEFT = 3;
#define DIR_RIGHT = 4;
#define DIR_NONE = 0;

byte addresses[][6] = {"1Node","2Node"};

RF24 radio(PIN_CE, PIN_CSN);

int X = 0;
int Y = 0;
float batteryVoltage;
float batteryPercentage;
bool batteryAlert = false;
byte batteryTestCnt = 0;
bool batteryMode = true;

void setup() 
{
  Serial.begin(115200);

  pinMode(PIN_PUSH, INPUT);  
  digitalWrite(PIN_PUSH,HIGH);
  pinMode(PIN_BTN_UP, INPUT);  
  digitalWrite(PIN_BTN_UP,HIGH);
  pinMode(PIN_BTN_DOWN, INPUT);  
  digitalWrite(PIN_BTN_DOWN,HIGH);
  pinMode(PIN_BTN_LEFT, INPUT);  
  digitalWrite(PIN_BTN_LEFT,HIGH);
  pinMode(PIN_BTN_RIGHT, INPUT);  
  digitalWrite(PIN_BTN_RIGHT,HIGH);
  Serial.println(F("JoyStick push buttons initialized..."));
  
  radio.begin();
  radio.openWritingPipe(addresses[1]); 
  Serial.println(F("Radio transmission line open..."));

  Wire.begin();
  delay(100);
  
  configMAX17043(32);
  qsMAX17043();
  Serial.println(F("I2C Battery Monitor initialized..."));

  // Disable trying to read battery data when serial available,
  // because there will not be a battery plugged in.
  if(batteryMode)
  {
    batteryPercentage = percentMAX17043();
    batteryVoltage = (float) vcellMAX17043() * 1/800;
    batteryAlert = batteryVoltage < 3.3;
  }

  if(batteryAlert)
  { 
    // TODO: Something
  }
}

byte pressed = 1;
byte up = 1;
byte down = 1;
byte left = 1;
byte right = 1;

void loop() 
{  
  X = analogRead(PIN_HORIZ);
  Y = analogRead(PIN_VERT);
  
  pressed = digitalRead(PIN_PUSH);
  up = digitalRead(PIN_BTN_UP);
  down = digitalRead(PIN_BTN_DOWN);
  left = digitalRead(PIN_BTN_LEFT);
  right = digitalRead(PIN_BTN_RIGHT);

  X = getSpeedFromAnalog(X, REST_X);
  Y = getSpeedFromAnalog(Y, REST_Y);

  char output[14];
  sprintf(output, "%s%03d%s%03d%s%s%s%s%s", 
    // X +/- and absolute speed value in three digits
    (X >= 0) ? "+" : "-", abs(X),
    // Y +/- and absolute speed value in three digits
    (Y <= 0) ? "+" : "-", abs(Y),
    // Send an X if the button is pressed, O for open
    (pressed == 0) ? "X" : "O",
    (up == 0) ? "X" : "O",
    (down == 0) ? "X" : "O",
    (left == 0) ? "X" : "O",
    (right == 0) ? "X" : "O");
    
  Serial.println(output);
  radio.write(&output, sizeof(output));  

  if(batteryTestCnt++ == 100 && batteryMode)
  {
    batteryPercentage = percentMAX17043();
    batteryVoltage = (float) vcellMAX17043() * 1/800;
    batteryAlert = batteryVoltage < 3.3;
    batteryTestCnt = 0;

    if(batteryAlert)
    { 
      // TODO: Perform a shutdown or alert action.
    }
  }
  
  delay(100);
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

/**
 * Returns a 12-bit ADC reading of the battery voltage, as reported by the MAX17043's VCELL register.
 * This does not return a voltage value. To convert this to a voltage, multiply by 5 and divide by 4096.
 */
unsigned int vcellMAX17043()
{
  unsigned int vcell;
  
  vcell = i2cRead16(0x02);
  vcell = vcell >> 4;  // last 4 bits of vcell are nothing
  
  return vcell;
}

/**
 * Returns a float value of the battery percentage reported from the SOC register of the MAX17043.
 */
float percentMAX17043()
{
  unsigned int soc;
  float percent;

  // Read SOC register of MAX17043.
  soc = i2cRead16(0x04);  

  // High byte of SOC is percentage.
  percent = (byte) (soc >> 8);  
  // Low byte is 1/256%.
  percent += ((float)((byte)soc))/256;  
  
  return percent;
}

/* *
 * Configures the config register of the MAX170143, specifically the alert threshold 
 * therein. Pass a  value between 1 and 32 to set the alert threshold to a value 
 * between 1 and 32%. Any other values will set the threshold to 32%.
 */
void configMAX17043(byte percent)
{
  // Anything 32 or greater will set to 32%.
  if ((percent >= 32)||(percent == 0))  
    i2cWrite16(0x9700, 0x0C);
  else
  {
    byte percentBits = 32 - percent;
    i2cWrite16((0x9700 | percentBits), 0x0C);
  }
}

/**
 * Issues a quick-start command to the MAX17043. A quick start allows the MAX17043 
 * to restart fuel-gauge calculations in the same manner as initial power-up of 
 * the IC. If an application's power-up sequence is very noisy, such that excess 
 * error is introduced into the IC's first guess of SOC, the Arduino can issue a 
 * quick-start to reduce the error.
 */
void qsMAX17043()
{
  // Write a 0x4000 to the MODE register.
  i2cWrite16(0x4000, 0x06);  
}

/**
 * Reads a 16-bit value beginning at the 8-bit address, and continuing to the next address. 
 * 16-bit value is returned.
 */
unsigned int i2cRead16(unsigned char address)
{
  int data = 0;
  
  Wire.beginTransmission(MAX17043_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(MAX17043_ADDRESS, 2);
  while (Wire.available() < 2)
    ;
  data = ((int) Wire.read()) << 8;
  data |= Wire.read();
  
  return data;
}

/**
 * Writes 16 bits of data beginning at an 8-bit address, and continuing to the next.
 */
void i2cWrite16(unsigned int data, unsigned char address)
{
  Wire.beginTransmission(MAX17043_ADDRESS);
  Wire.write(address);
  Wire.write((byte)((data >> 8) & 0x00FF));
  Wire.write((byte)(data & 0x00FF));
  Wire.endTransmission();
}
