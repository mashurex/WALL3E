# WALL3E Radio Receiver
This sketch is for an Arduino Uno to receive commands from a transmitter via an NRF24L01+ module and pass them to another micro controller over I2C. It is essentially an SPI to I2C bridge.

The reason this exists is because it was faster to do this than figure out how to use two components over SPI with a single Arduino, as the robot's motor control board uses SPI as well.