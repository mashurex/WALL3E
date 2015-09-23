
/**
 * WALL3E I2C controlled robot sketch for an Arduino Mega
 * Created by Mustafa Ashurex <ashurexm@gmail.com> 
 */
#include <AFMotor.h>
#include <Wire.h>

#define PIN_PROX_C 9
#define PIN_PROX_L_T 18
#define PIN_PROX_L_E 19
#define PIN_PROX_R_T 16
#define PIN_PROX_R_E 17
#define PIN_RDO_CS 7
#define PIN_RDO_CSN 8
#define I2C_ADDR 9
#define BUF_IDX_LR 0
#define BUF_IDX_LRSPEED 1
#define BUF_IDX_FB 4
#define BUF_IDX_FBSPEED 5
#define BUF_IDX_BTN 8

#define LEFT_SIDE 1
#define RIGHT_SIDE 2
#define RF 1
#define RR 4
#define LF 2
#define LR 3
#define TURNED_LEFT 1
#define TURNED_RIGHT 2
#define MIN_OBST_DIST 15
#define MAX_TURN_COUNT 2

byte addresses[][6] = {"1Node","2Node"};

// Right Side
AF_DCMotor mtrRF(RF); // RF
AF_DCMotor mtrRR(RR); // RR

// Left Side
AF_DCMotor mtrLF(LF); // LF
AF_DCMotor mtrLR(LR); // LR

// Detected forward obstacle count.
uint8_t obstacleCount = 0;

// Direction of the last turn.
uint8_t lastTurn = 0;

// The number of times turned in a single direction.
uint8_t turnCount = 0;

// Left obstacle count.
uint8_t leftObstCnt = 0;

// Right obstacle count.
uint8_t rightObstCnt = 0;

// Current forward distance reading.
long distance = 0;

// Current right distance reading.
long rDistance = 0;

// Current left distance reading.
long lDistance = 0;

uint8_t echoCount = 0;
bool lastActionComplete = true;

void setup() {
  Serial.begin(115200);

  motorInit(mtrRF);
  motorInit(mtrLF);
  motorInit(mtrRR);
  motorInit(mtrLR);
  Serial.println("Motors initialized...");
  
  // setSpeed(LEFT_SIDE, 255, FORWARD);
  // setSpeed(RIGHT_SIDE, 255, FORWARD);
  
  Wire.begin(I2C_ADDR);
  Wire.onReceive(receiveEvent);
  Serial.println(F("I2C wire initialized..."));
}

void loop() 
{
  // TODO: Proximity checking code.
  delay(10);
}

void initProximity(uint8_t pin)
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(2);
  digitalWrite(pin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pin, LOW);  
}

long proximityDistance(uint8_t trigger, uint8_t echo)
{  
  initProximity(trigger);
   
  pinMode(echo, INPUT);
  long duration = pulseIn(echo, HIGH);  
  long distance = msToCM(duration);
  
  return distance;
}

long msToCM(long ms)
{
  return ms/29/2;
}

void motorInit(AF_DCMotor motor)
{ 
  motor.setSpeed(200);
  motor.run(RELEASE);
}

void turnAround()
{    
  lastActionComplete = false;
  Serial.println(F("Turning around"));
  backward(255);
  
  int start = 0;
  while(start < 3000){ Serial.println(start); start++; }
   
  setSpeed(RIGHT_SIDE, 255, BACKWARD);
  setSpeed(LEFT_SIDE, 255, FORWARD);
  
  start = 0;
  while(start < 5500){ Serial.println(start); start++; }
  
  lastActionComplete = true; 
  Serial.println("Turned around!"); 
}

void turnLeft()
{
  Serial.println(F("Turning left"));
  setSpeed(RIGHT_SIDE, 255, FORWARD);
  setSpeed(LEFT_SIDE, 255, BACKWARD);
}

void turnRight()
{
  Serial.println(F("Turning right"));
  setSpeed(LEFT_SIDE, 255, FORWARD);
  setSpeed(RIGHT_SIDE, 255, BACKWARD);
}

void forward(uint8_t speedValue)
{  
  Serial.println(F("Going forward"));
  setSpeed(RIGHT_SIDE, speedValue, FORWARD);
  setSpeed(LEFT_SIDE, speedValue, FORWARD);
}

void backward(uint8_t speedValue)
{
  Serial.println(F("Going backward"));
  setSpeed(RIGHT_SIDE, speedValue, BACKWARD);
  setSpeed(LEFT_SIDE, speedValue, BACKWARD);
  delay(1500);
}

void stopMovement()
{
  Serial.println(F("Stopping"));
  setSpeed(RIGHT_SIDE, 0, FORWARD);
  setSpeed(LEFT_SIDE, 0, FORWARD);
}

void setSpeed(uint8_t pair, uint8_t speedValue, uint8_t cmd) 
{  
  if(pair == LEFT_SIDE) {
    mtrLF.setSpeed(speedValue);
    mtrLF.run(cmd);
    mtrLR.setSpeed(speedValue);
    mtrLR.run(cmd);
  }
  else if(pair == RIGHT_SIDE)
  {
    mtrRF.setSpeed(speedValue);
    mtrRF.run(cmd);
    mtrRR.setSpeed(speedValue);
    mtrRR.run(cmd);
  }
}

void getSpeedChars(char *input, char *output, uint8_t idx)
{
  for(uint8_t i = 0; i < 3; i++)
  {
    output[i] = input[idx + i];
  }
  output[3] = '\0';
}

int getSpeedFromBuffer(char *input, uint8_t idx)
{
  char chars[4];
  getSpeedChars(input, chars, idx);    
  return atoi(chars);
}

void handleRadioInput(char *input)
{       
    if(input[BUF_IDX_BTN] == 'X')
    {      
      turnAround();
      forward(255);
    }
    else
    {
      int xSpeed = getSpeedFromBuffer(input, BUF_IDX_LRSPEED);        
      int ySpeed = getSpeedFromBuffer(input, BUF_IDX_FBSPEED);    
      
      if(xSpeed < 100 && ySpeed < 100){ stopMovement(); }
      else if(xSpeed > ySpeed)
      {      
        if(input[BUF_IDX_LR] == '+'){ turnLeft(); }
        else { turnRight(); }           
      }
      else
      {
        if(input[BUF_IDX_FB] == '-'){ backward(ySpeed); }
        else{ forward(ySpeed); }      
      }
    }        
}

void receiveEvent(int bytes)
{
  if(lastActionComplete == true)
  {
    char wireBuffer[10] = {0};
    uint8_t idx = 0;
    uint8_t bufferSize = sizeof(wireBuffer);
    
    while(Wire.available())
    {
      if(idx < bufferSize)
      {
        wireBuffer[idx++] = Wire.read();
      }
    }

    handleRadioInput(wireBuffer);
  }  
}

