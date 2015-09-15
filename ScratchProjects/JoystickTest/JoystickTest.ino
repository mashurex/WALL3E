
#include <MicroView.h>

const uint8_t PIN_VERT = A1;
const uint8_t PIN_HORIZ = A2;
const uint8_t PIN_PUSH = 6;
const int REST_Y = 500;
const int REST_X = 512;
const int MAX_JOYSTICK = 1023;
const int MIN_JOYSTICK = 0;
const int FLT_JOYSTICK = 5;
const uint8_t MIN_SPEED = 0;
const uint8_t MAX_SPEED = 255;
const uint8_t DIR_FORWARD = 1;
const uint8_t DIR_BACKWARD = 2;
const uint8_t DIR_LEFT = 3;
const uint8_t DIR_RIGHT = 4;
const uint8_t DIR_NONE = 0;

MicroViewWidget *vertWidget;
MicroViewWidget *horzWidget;

void setup() {

  Serial.begin(9600);
  
  pinMode(PIN_PUSH, INPUT);  
  digitalWrite(PIN_PUSH,HIGH);
  
  uView.begin();
  uView.clear(PAGE); 
  vertWidget = new MicroViewGauge(15, 25, -255, 255);
  horzWidget = new MicroViewGauge(48, 25, -255, 255);
}

int X = 0;
int Y = 0;
uint8_t verticalDirection = DIR_NONE;
uint8_t horizontalDirection = DIR_NONE;
bool redraw = false;

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

    Serial.print("Speed value: ");
    Serial.println(speedValue);
    
    // Truncate to an integer to return a speed value between -255 and 255.
    return (int)speedValue;
  }
}

void loop() {
  X = analogRead(PIN_HORIZ);
  Y = analogRead(PIN_VERT);
  int pressed = digitalRead(PIN_PUSH);

  if(redraw){ uView.clear(PAGE); }

  X = getSpeedFromAnalog(X, REST_X);
  Y = getSpeedFromAnalog(Y, REST_Y);

  if(X > 0){ horizontalDirection = DIR_RIGHT; }  
  else if(X < 0){ horizontalDirection = DIR_LEFT; }

  if(Y > 0){ verticalDirection = DIR_FORWARD; }
  else if(Y < 0){ verticalDirection = DIR_BACKWARD; }  

  if(pressed != 1)
  {
    uView.rectFill(0,0,64,48);  
    redraw = true;
  }
  else
  {
    if(redraw)
    {
      vertWidget = new MicroViewGauge(15, 25, -255, 255);
      horzWidget = new MicroViewGauge(48, 25, -255, 255);      
    }
    vertWidget->setValue(X);
    horzWidget->setValue(Y);
    redraw = false;
  }
  uView.display();
  delay(1);
}
