//Pumpkin Light Show v1.0
//Michael Kukar
//October 2014
//Project #001

//This project was created to fit inside a 3D printed pumpkin and act as a flickering candle with sensor input
//e.g. it can also respond to sound and light input

/*
Takes in input from microphone, button, and light sensor
Outputs to two LEDs controlled over PWM

Button switches between the different states incrementally
State 1: 1 LED flickering
State 2: 2 LEDs flickering in sync
State 3: 2 LEDs react to music or sound
State 4: Lights only turn on when it is dark in the room, then uses state 2 logic (flickers both LEDs)

*/

//I/O pin variables
int led1 = 0; //pin 5 on ATTINY (PWM)
int led2 = 1; //pin 6 on ATTINY (PWM)
int button = 3; //pin 2 on ATTINY
int mic = 2; //pin 3 on ATTINY (analog)
int lightSense = 1;//Pin 7 on ATTINY (analog)

int state = 0; //saves the current state value

//LED variables
int brightness = 0; //saves the brightness of the LEDs

//microphone variables
int ambientAve = 0; //average ambient sound value
int rawSound = 0; //raw sound input from microphone
int rawAve = 0; //raw sound average over 4 values
int sound = 0; //sound value to use

//microphone variables
int soundAve = 0;
int soundCounter = 0;

//button variables
boolean previousHigh = false;

//light sensor variables
int rawLight = 0;
int light = 0;
int rawLightAve = 0;

void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(button, INPUT);
  randomSeed(analogRead(2)); //generates a random seed from pin 2 input
  ambientAve = ambientSoundVal(); //saves the average sound value of the ambience
  soundAve = ambientAve;
  delay(300); //delays enough to give the ambientSoundVal() function enough time to load values and everything to stabilize
}

void loop() {
  
  //debounces the button
  //When the button is pressed and THEN released, it increments the state on the release
  if (digitalRead(button) == HIGH) {
    //saves that the last state of the button was HIGH
    previousHigh = true;
  }
  else {
    //if the button was previously HIGH, or pressed, then it knows it was just released and increments the state
    if (previousHigh == true) {
      previousHigh = false;
      //increments the state (or loops around if it reaches the end)
      if (state == 2) {
        state = 0;
      }
      else {
        state++;
      }
    }
  }
  
  //determines the brightness randomly through the flickerVal() function
  brightness = flickerVal();
  
  //handles each state's logic
  switch(state) {
      
    case 0: //1 LEDs flickering all in sync
      //makes only the first light flicker and the other light off
      updateLights(brightness, 0);
      break;
    
    case 1: //2 LEDs flickering
    
      //makes both LEDs flicker
      updateLights(brightness, brightness);
      break;
    
    case 2: //music listener
    
      /*
      new idea for sound average. THIS WORKS DECENTLY, NOT GREAT
      1. calculate generic average sound value
      2. compare that sound value to average
      3. if the new sound is like, say 100 greater than the average, light up.
      4. otherwise, don't
      */
      
      /*
      rawSound = analogRead(mic);
      if ((rawSound - 300) > ambientAve) {
        digitalWrite(led1, HIGH);
        digitalWrite(led2, HIGH);
      }
      else if ((rawSound - 150) > ambientAve) {
        digitalWrite(led1, HIGH);
        digitalWrite(led2, LOW);
      }
      else {
        digitalWrite(led1, LOW);
        digitalWrite(led2, LOW);
      }
      
      */
      
      /*
      Alternate Idea:
      Running total of the ambient sound to get an accurate average
      If there is an outlyer, handle lighting up the LEDs
      If there isn't an outlier, then it should save the value in the running total
      When the running total matrix gets too large, then save the last average and go again (shouldn't have any wrap-around errors anyways)
      
      */
      
      rawSound = analogRead(mic);
      sound = rawSound - 200; //sound sets the boundary for a loud enough sound to justify turning on the lights
      
      if (sound > soundAve) {
        
        if ((sound - 150) > soundAve) {
          updateLights(255, 255);
        }
        else {
          updateLights(255, 0);
        }
        
      }
      //if the sound is not an outlier (ie it is not much greater than the average)
      //then sum the average and the new sound and divide by 2 to get the new average
      else {
        updateLights(0, 0);
        
        soundAve = (soundAve + rawSound)/2;
      }
      
      
      /*
      rawAve = 0;
      for(int i = 0; i < 4; i++) {
        rawSound = analogRead(mic);
        rawAve = rawAve + rawSound;
      }
      rawAve = rawAve/4;
      sound = abs(rawAve - 750);
      //uses different sound values to change the LEDs. Each LED has 4 brightness levels making 8 total states
      //LEVELS: 223 (2on 100%), 191(2on, 1 75%), 159(2on 1 50%), 127(1on 100%), 95(1on 75%), 63(1on 50%), 31, 0
      if (sound > 500) {
        analogWrite(led1, 255);
        analogWrite(led2, 255);
      }
      else if (sound > 400 && sound < 450) {
        analogWrite(led1, 255);
        analogWrite(led2, 192);
      }
      else if (sound > 300 && sound < 350) {
        analogWrite(led1, 255);
        analogWrite(led2, 128);
      }
      else if (sound > 200 && sound < 250) {
        analogWrite(led1, 255);
        analogWrite(led2, 0);
      }
      else if (sound > 100 && sound < 150) {
        analogWrite(led1, 192);
        analogWrite(led2, 0);
      }
      else if (sound < 50) {
        analogWrite(led1, 0);
        analogWrite(led2, 0);
      }
      
      */
      break;
      
    case 3: //only turns on in a dark room
      rawLight = analogRead(lightSense);
      if (rawLight > 600) {
        //makes the first two LEDs flicker
        updateLights(brightness, brightness);
      }
      else {
        //shuts off the lights when it is too bright
        updateLights(0, 0);
      }
      break;
     
  }
  
  //delays for a short time
  delay(20);
}

//outputs a random value for the flicker
int flickerVal() {
  int flickerVal = 255; //sets base rate to 255 (max value)
  flickerVal = flickerVal - random(160); //subtracts a random number from 1 - 160 (gives a nice contrast at this range)
  return flickerVal; //outputs the flicker value
}

//outputs the ambient sound level
int ambientSoundVal() {
  int ambientSoundVal = 0;
  int soundIn = 0;
  //sums the sound input and then divides it to get the average over 25 readings
  for(int i = 0; i < 25; i++) {
    soundIn = analogRead(mic);
    ambientSoundVal = ambientSoundVal + soundIn;
    delay(10);
  }
  ambientSoundVal = ambientSoundVal / 25; 
  return ambientSoundVal;
}

//Updates the LEDs in a convenient function
//L1_val is the brightness of LED1 (0 - 255) 
//L2_val is the brightness of LED2 (0 - 255)
//Will shut off the light if the value is outside the given range
void updateLights(int L1_val, int L2_val) {
  //makes sure the given values are between 0 - 255, then updates the corresponding LED
  //if it is outside of the range, it turns the LED off
  if (L1_val < 256 && L1_val >= 0) {
    analogWrite(led1, L1_val);
  }
  else {
    digitalWrite(led1, LOW);
  }
  
  if (L2_val < 256 && L2_val >= 0) {
    analogWrite(led2, L2_val);
  }
  else {
    digitalWrite(led2, LOW);
  }
}
