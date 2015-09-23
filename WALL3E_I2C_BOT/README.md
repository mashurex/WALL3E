# WALL3E I2C Controlled Bot
This sketch is for an Arduino Mega to receive commands from an Uno over an I2C bridge.

The Mega received command events from the I2C channel and then decodes the payload into instructions such as the direction and speed to move in.

## Notes
This code is currently very responsive but crashy- frequently it gets stuck moving in a direction and quits responding to events. This is likely due to the fake delay while loop counter.