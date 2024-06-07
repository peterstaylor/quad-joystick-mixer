# quad-joystick-mixer
This is a README for the quadrophonic joystick mixer design

# Hardware
## V1 to V2 Change Log: 
- [] Change input gain-setting pots to A50k from A500k
- [] Add analog filtering and buffering to the input path from the encoder to smoothe out readings
- [] Add six more analog inputs for a total of 8 inputs 
- [] Perform electromechanical integration work once the design is working

# Firmware
## Teensy 
- Link to Teensy tutorial: https://www.pjrc.com/teensy/tutorial.html
- Link to how to set up a portable install of the Arduino IDE: https://docs.arduino.cc/software/ide-v1/tutorials/PortableIDE/
    - I had to set up a portable install to get all the teensy libraries to install correctly and compile the code

## Board Level Test (BLT) Code
This code is written to exercise the I/O on the system in a simple way just to verify the hardware

### Major Board Level Test Functions: 
- checkBLTTransition(): 
    - This function looks for the user to press down button 1 for a certain amount of time and then puts the board in BLT mode
- flash(): 
    - flashes both LEDs the desired number of times
    - input argument is an int setting the number of flashes
- analogReadToChannelCount: 
    - input argument is an int return from reading the encoder ADC channel
    - output is the channel of the mixer 1 through 8
    - use this function to determine what channel the user is trying to control
- binaryLED: 
    - input is int of channel count 
    - i used this for BLT to verify i was iterating through the channels visually
- calibrateJoystick: 
    - I'm not using this function right now but will probably be useful for application as I've already seen that there are inconsistencies between the two joysticks I have