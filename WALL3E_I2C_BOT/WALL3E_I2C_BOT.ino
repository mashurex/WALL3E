
/**
 * WALL3E I2C controlled robot sketch for an Arduino Mega
 * 
 * Created by Mustafa Ashurex <ashurexm@gmail.com> 
 */
#include <AFMotor.h>
#include <Wire.h>

// Proximity pins.
#define PIN_PROX_C 9
#define PIN_PROX_L_T 18
#define PIN_PROX_L_E 19
#define PIN_PROX_R_T 16
#define PIN_PROX_R_E 17

// Radio pins.
#define PIN_RDO_CS 7
#define PIN_RDO_CSN 8

// The I2C address for this microcontroller.
#define I2C_ADDR 9

// Radio command buffer indices.
#define BUF_IDX_LR 0
#define BUF_IDX_LRSPEED 1
#define BUF_IDX_FB 4
#define BUF_IDX_FBSPEED 5
#define BUF_IDX_BTN 8
#define BUF_IDX_UP 9
#define BUF_IDX_DOWN 10
#define BUF_IDX_LEFT 11
#define BUF_IDX_RIGHT 12
#define ON_CHAR 'X'

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

// When a command is being performed this will be false.
bool lastActionComplete = true;

// Command buffer data received from radio transmissions.
char commandBuffer[14] = {0};
// Will be true when a command has been received and saved to the command buffer.
bool commandWaiting = false;

void setup() 
{
  Serial.begin(115200);

  motorInit(mtrRF);
  motorInit(mtrLF);
  motorInit(mtrRR);
  motorInit(mtrLR);
  Serial.println(F("Motors initialized..."));  
  
  Wire.begin(I2C_ADDR);  
  Wire.onReceive(receiveEvent);
  Serial.println(F("I2C wire initialized..."));
  Serial.println(F("Ready!"));
}

void loop() 
{
  if(commandWaiting)  
  {
    handleRadioInput(commandBuffer); 
  }
}

void handleRadioInput(char *input)
{      
    commandStarted();
     
    if(input[BUF_IDX_BTN] == ON_CHAR)
    {      
      turnAround();      
    }    
    else
    {
      // TODO: Not using digital buttons on controller yet, but let them work
      // in conjunction w/ the joystick.
      if(input[BUF_IDX_UP] == ON_CHAR)
      {
        Serial.println(F("Up button pressed."));
      }
      else if(input[BUF_IDX_DOWN] == ON_CHAR)
      {
        Serial.println(F("Down button pressed."));
      }
      else if(input[BUF_IDX_LEFT] == ON_CHAR)
      {
        Serial.println(F("Left button pressed."));
      }
      else if(input[BUF_IDX_RIGHT] == ON_CHAR)
      {
        Serial.println(F("Right button pressed."));
      }
      
      int xSpeed = getSpeedFromBuffer(input, BUF_IDX_LRSPEED);        
      int ySpeed = getSpeedFromBuffer(input, BUF_IDX_FBSPEED);    

      // If the speed is sufficiently slow, just stop.
      if(xSpeed < 80 && ySpeed < 80){ stopMovement(); }
      // If the left or right speed is greater than forward/rear speed perform a turn.
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
    commandCompleted(); 
}

/**
 * Sets radio input/command flags as necessary.
 */
void setRadioInput()
{  
  lastActionComplete = false;  
  commandWaiting = true;
}

/**
 * Sets command/buffer flags to completed states.
 */
void commandCompleted()
{
  lastActionComplete = true;
  commandWaiting = false;
}

/**
 * Sets command/buffer flags to working states.
 */
void commandStarted()
{  
  lastActionComplete = false;
  commandWaiting = false;
}

/**
 * Initializes the proximity sensor for a given pin.
 */
void initProximity(uint8_t pin)
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(2);
  digitalWrite(pin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pin, LOW);  
}

/**
 * Returns the proximity sensor distance for a given set of pins.
 */
long proximityDistance(uint8_t trigger, uint8_t echo)
{  
  initProximity(trigger);
   
  pinMode(echo, INPUT);
  long duration = pulseIn(echo, HIGH);  
  long distance = msToCM(duration);
  
  return distance;
}

/**
 * Converts a millisecond proximity reading to a distance in centimeters.
 */
long msToCM(long ms)
{
  return ms/29/2;
}

/**
 * Initialize a specified motor.
 */
void motorInit(AF_DCMotor motor)
{ 
  motor.setSpeed(200);
  motor.run(RELEASE);
}

/**
 * Peform a series of steps to turn around.
 */
void turnAround()
{      
  Serial.println(F("Turning around"));
  backward(255);
  delay(1000);
   
  setSpeed(RIGHT_SIDE, 255, BACKWARD);
  setSpeed(LEFT_SIDE, 255, FORWARD); 
  delay(2000);

  setSpeed(RIGHT_SIDE, 0, RELEASE);
  setSpeed(LEFT_SIDE, 0, RELEASE);
  Serial.println(F("Turned around!")); 
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
}

void stopMovement()
{
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

/**
 * Get the speed data from input characters at a given index.
 */
void getSpeedChars(char *input, char *output, uint8_t idx)
{
  for(uint8_t i = 0; i < 3; i++)
  {
    output[i] = input[idx + i];
  }
  output[3] = '\0';
}

/**
 * Return a specific speed value from the input buffer.
 */
int getSpeedFromBuffer(char *input, uint8_t idx)
{
  char chars[4];
  getSpeedChars(input, chars, idx);    
  return atoi(chars);
}

/**
 * Process a radio input event.
 */
void receiveEvent(int bytes)
{
  // TODO: Command waiting may not be necessary.
  bool doWork = (lastActionComplete == true && !commandWaiting);
  
  uint8_t idx = 0;
  uint8_t bufferSize = sizeof(commandBuffer);
  
  while(Wire.available())
  {
    if(idx < bufferSize && doWork)
    {      
        commandBuffer[idx++] = Wire.read();
    } 
    // Read the wire but don't do anything with it.
    else { Wire.read(); }
  }
    
  if(doWork)
  {
    Serial.println(commandBuffer);        
    setRadioInput();
  }   
}



