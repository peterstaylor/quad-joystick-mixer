#include <Wire.h>

// variables
int RED = 2;
int BLUE = 3;
int button1 = 7;
int button2 = 8;
int button3 = 6;

// channel selection variables for encoder
int encoderInput = 14;
int channelSelect = 0;

// joystick management variables
bool calibrateJoystick = false; 
int ABcontrol = 15;
int ABreading = 0;
int CDcontrol = 20;
int CDreading = 0;
int ABmax = 512;
int ABmin = 512;
int ABmid = 512;
int CDmax = 512;
int CDmin = 512;
int CDmid = 512;

// channel selection analog reading cutoffs
int channel1Cutoff = 190;
int channel2Cutoff = 319;
int channel3Cutoff = 448;
int channel4Cutoff = 574;
int channel5Cutoff = 702;
int channel6Cutoff = 832;
int channel7Cutoff = 959;

// variables for checking button 1
// used to transition to BLT mode
bool runBLT = false;
unsigned long startBLTdet = 0;
unsigned long endBLTdet = 0;
unsigned long bltOnTimeout = 2000;

// button state management
int button1State = 0;
int lastButton1State = 0;
int button2State = 0;
int lastButton2State = 0;
int button3State = 0;
int lastButton3State = 0;

// digital pot variables
char channel1ABaddr = 0x28; 
char channel1CDaddr = 0x29; 
char channel2ABaddr = 0x28; 
char channel2CDaddr = 0x29; 
// "max" volume is lowest value, "min" volume is highest value
int pot0max = 0;
int pot0mid = 31; // <-- default when joystick is centered?
int pot0min = 63; // mute setting
int pot1max = 64; 
int pot1mid = 95; // <-- default when joystick is centered? 
int pot1min = 127; // mute setting
// see datasheet for details on config but: 
// pot values are stored in NVM
// zero crossing is enabled
// pot is set for 63 settings
char potConfig = 0b10000010; 

// setup function
void setup() {
  Serial.begin(38400);
  Serial.println("Beginning setup...");

  // red and blue LEDs can be used for status indication in the system
  pinMode(RED, OUTPUT);
  pinMode(BLUE, OUTPUT);

  // start I2C peripherals
  Wire.begin(); 
  Wire1.begin();   

  Serial.println("Setup complete...");
}


void loop() {
  if (runBLT) {
    boardLevelTest();
    runBLT = false;
  } else {
    checkBLTtransition();
    // app code goes here
  }
}

// implmenets a timeout on button 1
// if you hold down button 1 long enough, the system will switch to BLT mode
void checkBLTtransition() {
  button1State = digitalRead(button1);
  if (button1State != lastButton1State) {
    if (button1State == 0) {
      startBLTdet = millis();
    } else {
      endBLTdet = millis();
      if (endBLTdet - startBLTdet > bltOnTimeout) {
        runBLT = true;
      }
      endBLTdet = 0;
      startBLTdet = 0;
    }
  }
  lastButton1State = button1State;
}

void flash(int numFlash) {
  for (int i = 0; i < numFlash; i++) {
    digitalWrite(RED, HIGH);
    digitalWrite(BLUE, HIGH);
    delay(500);
    digitalWrite(RED, LOW);
    digitalWrite(BLUE, LOW);
    delay(500);
  }
  return;
}

int analogReadToChannelCount(int analogRead) {
  int channelCount;
  if (analogRead < channel1Cutoff) {
    channelCount = 1;
  } else if (analogRead < channel2Cutoff) {
    channelCount = 2;
  } else if (analogRead < channel3Cutoff) {
    channelCount = 3;
  } else if (analogRead < channel4Cutoff) {
    channelCount = 4;
  } else if (analogRead < channel5Cutoff) {
    channelCount = 5;
  } else if (analogRead < channel6Cutoff) {
    channelCount = 6;
  } else if (analogRead < channel7Cutoff) {
    channelCount = 7;
  } else {
    channelCount = 8;
  }

  return channelCount;
}

void binaryLED(int count) {
  digitalWrite(RED, LOW);
  digitalWrite(BLUE, LOW);
  if (count == 1 || count == 5) {
    digitalWrite(RED, LOW);
    digitalWrite(BLUE, LOW);
  } else if (count == 2 || count == 6) {
    digitalWrite(RED, HIGH);
    digitalWrite(BLUE, LOW);
  } else if (count == 3 || count == 7) {
    digitalWrite(RED, LOW);
    digitalWrite(BLUE, HIGH);
  } else if (count == 4 || count == 8) {
    digitalWrite(RED, HIGH);
    digitalWrite(BLUE, HIGH);
  }
  return;
}

void boardLevelTest() {
  Serial.println("Beginning test procedure...");
  // Test 1, basic blink pattern on two LEDs
  //test1();

  // Test 2, check buttons 2 and 3
  //test2();

  // Test 3, checks the encoder
  //test3();

  // Test 4 checks the joystick input
  //test4();

  // Test 5 checks the I2c comms with the pots
  test5(); 
}

// first test just toggles the red and blue LEDs to verify hardware connection
void test1() {
  bool proceed = false;
  Serial.println("Beginning test #1:");
  flash(1);
  Serial.println("Red and blue LEDs should be flashing quickly and opposite from each other");
  Serial.println("Press button 1 to proceed");
  while (!proceed) {
    digitalWrite(RED, HIGH);
    digitalWrite(BLUE, LOW);
    delay(100);
    digitalWrite(RED, LOW);
    digitalWrite(BLUE, HIGH);
    delay(100);
    if (digitalRead(button1) == LOW) {
      proceed = true;
      Serial.println("Proceeding to next test");
    }
  }

  // reset everything after test 1
  digitalWrite(RED, LOW);
  digitalWrite(BLUE, LOW);
}

// second test uses button 2 and 3 to toggle the LEDs, verifying the buttons
void test2() {
  bool proceed = false;
  Serial.println("Beginning test #2:");
  flash(2);
  Serial.println("Press button 2 to toggle red LED, button 3 to toggle blue LED");
  Serial.println("Press button 1 to proceed");
  while (!proceed) {
    button2State = digitalRead(button2);
    if (button2State != lastButton2State && button2State == 0) {
      digitalWrite(RED, !digitalRead(RED));
    }
    lastButton2State = button2State;

    button3State = digitalRead(button3);
    if (button3State != lastButton3State && button3State == 0) {
      digitalWrite(BLUE, !digitalRead(BLUE));
    }
    lastButton3State = button3State;

    if (digitalRead(button1) == LOW) {
      proceed = true;
      Serial.println("Proceeding to next test");
    }
  }

  // reset everything after test 2
  digitalWrite(RED, LOW);
  digitalWrite(BLUE, LOW);
}

// check the encoder
void test3() {
  int channelCount = 0;
  int lastChannelCount = 0;
  bool proceed = false;
  int repCount = 0;
  int repCutoff = 5000;
  Serial.println("Beginning test 3:");
  flash(3);
  Serial.println("Rotate encoder to cycle through binary states on LEDs");
  Serial.println("Press button 1 to proceed");
  while (!proceed) {

    channelSelect = analogRead(encoderInput);
    channelCount = analogReadToChannelCount(channelSelect);
    if (channelCount == lastChannelCount) {
      repCount++;
    } else {
      repCount = 0;
    }
    if (repCount >= repCutoff) {
      binaryLED(channelCount);
    }
    lastChannelCount = channelCount;

    if (digitalRead(button1) == LOW) {
      proceed = true;
      Serial.println("Proceeding to next test");
    }
  }
}

void test4() {
  bool proceed = false;
  Serial.println("Beginning test #4"); 
  flash(4);
  if (calibrateJoystick) {
    joystickCalibration(); 
  }

  proceed = false;
  delay(1000);
  while (!proceed) {
    ABreading = int(float(analogRead(ABcontrol)) / 4.0);
    CDreading = int(float(analogRead(CDcontrol)) / 4.0);
    analogWrite(RED, ABreading);
    analogWrite(BLUE, CDreading);
    if (digitalRead(button1) == LOW) {
      proceed = true;
      analogWrite(RED, 0); 
      analogWrite(BLUE, 0); 
      Serial.println("Moving on to next test");  //not using calibrations for now, just put in to test
    }
  }
}

void joystickCalibration() {
  bool proceed = false; 
  Serial.println("let joystick rest in middle to calibrate center measurement:");
  delay(1000);
  for (int i = 0; i < 10000; i++) {
    ABmid = (ABmid + analogRead(ABcontrol)) / 2;
    CDmid = (CDmid + analogRead(CDcontrol)) / 2;
  }
  Serial.println(ABmid);
  Serial.println(CDmid);
  Serial.println("middle calibrated");
  Serial.println("now swing joystick in full circle for a few seconds");
  Serial.println("press button 1 to proceed after swinging joystick around");
  while (!proceed) {
    ABreading = analogRead(ABcontrol);
    CDreading = analogRead(CDcontrol);
    if (ABreading > ABmax) {
      ABmax = ABreading;
    } else if (ABreading < ABmin) {
      ABmin = ABreading;
    }

    if (CDreading > CDmax) {
      CDmax = CDreading;
    } else if (CDreading < CDmin) {
      CDmin = CDreading;
    }
    if (digitalRead(button1) == LOW) {
      proceed = true;
      Serial.println("Calibration complete!");  //not using calibrations for now, just put in to test
    }
  }
}

// try to talk to the digital pots
void test5() {
  bool proceed = false;
  int output = 0; 
  Serial.println("Beginning test #5:");
  flash(5);
  initialI2CCheck(); 
  while (!proceed) {
    toggleOutputs(output); 
    output++; 
    if(output == 3){
      output = 0; 
    }
    if (digitalRead(button1) == LOW) {
      proceed = true;
      Serial.println("Moving on to next test");  //not using calibrations for now, just put in to test
    }
  }
}

void toggleOutputs(int output) {
  if (output == 0) {
    Wire.beginTransmission(channel1ABaddr);
    Wire.write(pot0max);  // turn on A
    Wire.write(pot1min);  // mute B
    Wire.endTransmission();
    Wire.beginTransmission(channel1CDaddr);
    Wire.write(pot0min);  // mute C
    Wire.write(pot1min);  // mute D
    Wire.endTransmission(); 

    Wire1.beginTransmission(channel2ABaddr); 
    Wire1.write(pot0max); // turn on A
    Wire1.write(pot1min);  // mute B
    Wire1.endTransmission(); 
    Wire1.beginTransmission(channel2CDaddr); 
    Wire1.write(pot0min); // mute C
    Wire1.write(pot1min); // mute D
    Wire1.endTransmission(); 

  } else if (output == 1) {
    Wire.beginTransmission(channel1ABaddr);
    Wire.write(pot0min);  
    Wire.write(pot1max);  
    Wire.endTransmission();
    Wire.beginTransmission(channel1CDaddr);
    Wire.write(pot0min);  
    Wire.write(pot1min);  
    Wire.endTransmission(); 

    Wire1.beginTransmission(channel2ABaddr); 
    Wire1.write(pot0min); 
    Wire1.write(pot1max);  
    Wire1.endTransmission(); 
    Wire1.beginTransmission(channel2CDaddr); 
    Wire1.write(pot0min); 
    Wire1.write(pot1min); 
    Wire1.endTransmission();

  } else if (output == 2) {
    Wire.beginTransmission(channel1ABaddr);
    Wire.write(pot0min);  
    Wire.write(pot1min);  
    Wire.endTransmission();
    Wire.beginTransmission(channel1CDaddr);
    Wire.write(pot0max);  
    Wire.write(pot1min);  
    Wire.endTransmission(); 

    Wire1.beginTransmission(channel2ABaddr); 
    Wire1.write(pot0min); 
    Wire1.write(pot1min);  
    Wire1.endTransmission(); 
    Wire1.beginTransmission(channel2CDaddr); 
    Wire1.write(pot0max); 
    Wire1.write(pot1min); 
    Wire1.endTransmission();

  } else if (output == 3) {
    Wire.beginTransmission(channel1ABaddr);
    Wire.write(pot0min);  
    Wire.write(pot1min);  
    Wire.endTransmission();
    Wire.beginTransmission(channel1CDaddr);
    Wire.write(pot0min);  
    Wire.write(pot1max);  
    Wire.endTransmission(); 

    Wire1.beginTransmission(channel2ABaddr); 
    Wire1.write(pot0min); 
    Wire1.write(pot1min);  
    Wire1.endTransmission(); 
    Wire1.beginTransmission(channel2CDaddr); 
    Wire1.write(pot0min); 
    Wire1.write(pot1max); 
    Wire1.endTransmission();
  }
  delay(500); 
  return;
}

void configI2C(){
  Wire.beginTransmission(channel1ABaddr); 
  Wire.write(potConfig); 
  Wire.endTransmission(); 

  Wire.beginTransmission(channel1CDaddr); 
  Wire.write(potConfig); 
  Wire.endTransmission(); 

  Wire1.beginTransmission(channel2ABaddr); 
  Wire1.write(potConfig); 
  Wire1.endTransmission(); 

  Wire1.beginTransmission(channel2CDaddr); 
  Wire1.write(potConfig); 
  Wire1.endTransmission(); 

  return; 
}

void initialI2CCheck() {
  int testSum = 0; 

  Wire.requestFrom(channel1ABaddr, 3);
  while (Wire.available()) {
    testSum += int(Wire.read()); 
  }
  if(testSum != 325){
    Serial.println("Channel 1 AB Failed"); 
    return; 
  }

  testSum = 0; 
  Wire.requestFrom(channel1CDaddr, 3);
  while (Wire.available()) {
    testSum += int(Wire.read()); 
  }
  if(testSum != 325){
    Serial.println("Channel 1 CD Failed"); 
    return; 
  }

  testSum = 0; 
  Wire1.requestFrom(channel2ABaddr, 3);
  while (Wire1.available()) {
    testSum += int(Wire1.read()); 
  }
  if(testSum != 325){
    Serial.println("Channel 2 AB Failed"); 
    return; 
  }
  
  testSum = 0; 
  Wire1.requestFrom(channel2CDaddr, 3);
  while (Wire1.available()) {
    testSum += int(Wire1.read()); 
  }
  if(testSum != 325){
    Serial.println("Channel 2 CD Failed"); 
    return; 
  }
  Serial.println("All digital pots passed initial check!"); 
  return; 
}
