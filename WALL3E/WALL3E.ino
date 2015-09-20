#include <AFMotor.h>
#include <Wire.h>

#define I2C_ADDR 9

const uint8_t LEFT_SIDE = 1;
const uint8_t RIGHT_SIDE = 2;
const uint8_t RF = 1;
const uint8_t RR = 4;
const uint8_t LF = 2;
const uint8_t LR = 3;
const uint8_t PIN_PROX_C = 9;
const uint8_t PIN_PROX_L_T = 18;
const uint8_t PIN_PROX_L_E = 19;
const uint8_t PIN_PROX_R_T = 16;
const uint8_t PIN_PROX_R_E = 17;
const uint8_t TURNED_LEFT = 1;
const uint8_t TURNED_RIGHT = 2;
const uint8_t MIN_OBST_DIST = 15;
const uint8_t MAX_TURN_COUNT = 2;

// Right Side
AF_DCMotor mtrRF(RF); // RF
AF_DCMotor mtrRR(RR); // RR

// Left Side
AF_DCMotor mtrLF(LF); // LF
AF_DCMotor mtrLR(LR); // LR

// Detected forward obstacle count.
uint8_t obstacle_cnt = 0;

// Direction of the last turn.
uint8_t last_turn = 0;

// The number of times turned in a single direction.
uint8_t turn_count = 0;

// Left obstacle count.
uint8_t l_distance_cnt = 0;

// Right obstacle count.
uint8_t r_distance_cnt = 0;

// Current forward distance reading.
long distance = 0;

// Current right distance reading.
long r_distance = 0;

// Current left distance reading.
long l_distance = 0;

void setup() 
{
  Serial.begin(9600);

  // Initialize the motors and start moving forward.
  motorInit(mtrRF);
  motorInit(mtrLF);
  motorInit(mtrRR);
  motorInit(mtrLR);
  
  setSpeed(LEFT_SIDE, 255, FORWARD);
  setSpeed(RIGHT_SIDE, 255, FORWARD);  
}

void loop() 
{
  distance = proximityDistance(PIN_PROX_C, PIN_PROX_C);
  r_distance = proximityDistance(PIN_PROX_R_T, PIN_PROX_R_E);
  l_distance = proximityDistance(PIN_PROX_L_T, PIN_PROX_L_E);

  // Throw away 0 distance readings, they're invalid.
  if(distance == 0)
  {
    Serial.println("Bed center proximity scan.");  
  }
  else if(distance < MIN_OBST_DIST)
  {
    Serial.print(distance);
    Serial.println("cm from obstactle");
    obstacle_cnt++;
  }
  // No obstacles, reset counters.
  else 
  { 
    obstacle_cnt = 0; 
    turn_count = 0;
  }

  // Check left distance first, err on the side of turning to the right from obstacles being approached.
  // TODO: Check both sides and determine whether or not to stay straight or turn away from the closest
  // upcoming obstacle.
  if(l_distance < MIN_OBST_DIST && l_distance > 0)
  { 
    // Turn away from an obstacle if detected numerous times already.
    if(l_distance_cnt > 2)
    {
      Serial.println("Avoiding obstacle on left.");
      turnRight(); 
      l_distance_cnt = 0;
    } else { l_distance_cnt++; }
  }  
  else if(r_distance < 15 && r_distance > 0)
  { 
    if(r_distance_cnt > 2)
    {
      Serial.println("Avoiding obstacle on right.");
      turnLeft(); 
      r_distance_cnt = 0;
    } else { r_distance_cnt++; }
  }
  // No obstacles were found, reset obstacle state.
  else
  {
    l_distance_cnt = 0;
    r_distance_cnt = 0;    
    last_turn = 0;
  }

  // If a forward obstacle is detected numerous times, back away and determine whether to go left or right.
  if(obstacle_cnt > 3)
  {
    // Backup before turning right or left.
    backward(255);
    
    long r_dist = proximityDistance(PIN_PROX_R_T, PIN_PROX_R_E);
    long l_dist = proximityDistance(PIN_PROX_L_T, PIN_PROX_L_E);

    delayMicroseconds(2);

    // Take a number of readings to get more precise data, throw away bad reads.    
    for(uint8_t i = 0; i < 4; i++)
    {      
      // Read right proximity.
      long tmp_r = proximityDistance(PIN_PROX_R_T, PIN_PROX_R_E);         
      // Throw away zero values, they were a bad read.
      if(tmp_r > 0)
      {
        // Average the distances as they come in.
        r_dist = (r_dist + tmp_r) / 2;
      }
      // Read left proximity.
      long tmp_l = proximityDistance(PIN_PROX_L_T, PIN_PROX_L_E);
      if(tmp_l > 0)
      {
        l_dist = (l_dist + tmp_l) / 2;
      }
      // Delay briefly to allow a better read.
      delayMicroseconds(10);
    }

    // Delay a bit before turning to allow for a farther backup.
    delay(500);

    // If the left appears more/as clear, go left.
    if(l_dist >= r_dist)
    {
      if(last_turn == TURNED_LEFT)
      {
        // If turning the same direction repeatedly doesn't unblock, turn around.
        if(turn_count > MAX_TURN_COUNT)
        { 
          turnAround(); 
          // Reset turn count after turning around.
          turn_count = 0;
        }
        else { turn_count++; }
      }
      else
      {
        last_turn = TURNED_LEFT;        
        turn_count = 1;
        // Turn left        
        turnLeft();
        // Go forward in a left direction, briefly.
        forward(200);        
        delay(100);
        // Turn right to straighten out a bit.
        turnRight();  
      }        
    }
    // Otherwise go right.
    else
    {      
      if(last_turn == TURNED_RIGHT)
      {
        if(turn_count > MAX_TURN_COUNT)
        { 
          turnAround(); 
          turn_count = 0;
        }
        else { turn_count++; }
      }
      else
      {
        last_turn = TURNED_RIGHT;        
        turn_count = 1;        
        turnRight();
        forward(200);
        delay(100);
        turnLeft(); 
      }        
    }
    // Reset the obstacle count after avoiding.
    obstacle_cnt = 0;    
  }  
  
  forward(255);  
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
  if(duration == 0)
  {
    delayMicroseconds(10);
    initProximity(trigger);
    duration = pulseIn(echo, HIGH);
  }
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
  backward(255);
  delay(700);
  setSpeed(RIGHT_SIDE, 255, BACKWARD);
  delay(1500);
}

void turnLeft()
{
  setSpeed(LEFT_SIDE, 255, BACKWARD);
  delay(500);
  setSpeed(LEFT_SIDE, 255, FORWARD);
}

void turnRight()
{
  setSpeed(RIGHT_SIDE, 255, BACKWARD);
  delay(500);
  setSpeed(RIGHT_SIDE, 255, FORWARD);
}

void forward(uint8_t speedValue)
{  
  setSpeed(RIGHT_SIDE, speedValue, FORWARD);
  setSpeed(LEFT_SIDE, speedValue, FORWARD);
}

void backward(uint8_t speedValue)
{
  setSpeed(RIGHT_SIDE, speedValue, BACKWARD);
  setSpeed(LEFT_SIDE, speedValue, BACKWARD);
}

void setSpeed(uint8_t pair, uint8_t speedValue, uint8_t cmd) {  
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

