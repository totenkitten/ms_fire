# ms_fire
Control code for Ms. Fire. This is a basic differential drive platform with standard RC control, for Arduino.

The RC receiver input code is from: http://drandom.blogspot.com/2013/04/v-tail-mixer-with-low-pass-filter.html

This code also adds a check on the last time since a start pulse was received from the RX to act as a hearbeat/estop. 
Without this, failure of the transmitter results in full speed activation of both motors, which ranges from "bad" in lightweight machines to "potentially fatal" in heavy ones. 
If this is a concern, why _the fuck_ are you using control code you _found on the internet_ to operate _heavy machinery_. You May Very Well Die. 

The resulting values are used to control high current H-bridge modules as described here: http://www.hessmer.org/blog/2013/12/28/ibt-2-h-bridge-with-arduino/

There are other ways the motor driver could be wired up, this one works fine.