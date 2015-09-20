/**
 * WALL3E_RX
 * Robot NRF240L1 receiving decoder.
 * Code for an Arduino Uno connected to an NRF240L1 receiver that will
 * receive instructions from a transmitter and send them to another 
 * microcontroller over I2C.
 */
#include <SPI.h>
#include <RF24.h>
#include <Wire.h>

// NRF24L01 comms pins.
#define PIN_RDO_CS 7
#define PIN_RDO_CSN 8

// I2C transmission address.
#define I2C_ADDR 9

// Buffer index position to check for message validity.
#define BUF_IDX_CHECK 0

RF24 radio(PIN_RDO_CS,PIN_RDO_CSN);
byte addresses[][6] = {"1Node","2Node"};

void setup() 
{
  Serial.begin(115200);

  Wire.begin();
  Serial.println(F("I2C wire started..."));
  
  radio.begin();
  radio.openReadingPipe(1,addresses[1]);
  radio.startListening();
  Serial.println(F("Radio listening..."));
}

char radioBuffer[10] = {0};
bool isListeningStatus = true;

void loop() 
{
    
  if(radio.available())
  {    
    radio.read(&radioBuffer, sizeof(radioBuffer));
    
    if(radioBuffer[BUF_IDX_CHECK] != '+' && radioBuffer[BUF_IDX_CHECK] != '-')
    {
      Serial.print(F("Bad read: "));
      Serial.println(radioBuffer);
        
    }
    else
    {  
      stopListening();
      
      // Proxy the radio buffer to the I2C channel.
      Wire.beginTransmission(I2C_ADDR);
      Wire.write(radioBuffer);
      Wire.endTransmission();

      Serial.println(radioBuffer);
    }
  }
  
  if(!isListening()){ startListening(); }
  delay(10);
}

void stopListening()
{
  radio.stopListening();
  isListeningStatus = false;
}

void startListening()
{
  radio.startListening();
  isListeningStatus = true;
}

bool isListening(){ return isListeningStatus; }

