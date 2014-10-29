//Pumpkin Light Show v1.1
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
State 3: 2 LEDs react to music or sound (makes both LEDs on HIGH)
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

//new microphone variables
int runningAve = 0;
int startupCount = 10;
int peakHeight = 200;

//button variables
boolean previousHigh = false;

//light sensor variables
int rawLight = 0;
int light = 0;
int rawLightAve = 0;
int flickerTimer = 0;

void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(button, INPUT);
  randomSeed(analogRead(2)); //generates a random seed from pin 2 input
  delay(250); //delays a slight bit for startup
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
      if (state == 3) {
        state = 0;
      }
      else {
        state++;
      }
    }
  }
  
  //determines the brightness randomly through the flickerVal() function
  //only updates every 12 * 5ms = 60ms for an even, candle-like flicker (if its too fast it looks artificial)
  if (flickerTimer < 12) {
    flickerTimer = flickerTimer + 1;
  }
  else {
    flickerTimer = 0;
    brightness = flickerVal();
  }
  
  //handles each state's logic
  switch(state) {
      
    case 0: //1 LEDs flickering all in sync
      //makes only the first light flicker and the other light off
      updateLights(brightness, 0);
      break;
    
    case 1: //2 LEDs flickering
    
      //makes both LEDs flicker
      updateLights(brightness, brightness);
      
      //resets the state variables for the music listener startup sequence
      startupCount = 10;
      break;
    
    case 2: //music listener

      rawSound = abs(analogRead(mic)); //reads in the raw sound input
      if (rawSound > peakHeight) { //if the sound is greater than the peak height it will subtract, otherwise the sound is too low to use
        sound = rawSound - peakHeight;
      }
      else {
        sound = 0;
      }
      //if it is no longer starting up go here
      if (startupCount == 0) {
        if (sound >= runningAve) { //if the rawSound - peakHeight is greater than the average, then it turns on the lights
          updateLights(255, 255);
        }
        else { //otherwise it shuts off the lights
          updateLights(0,0);
        }
        runningAve = (rawSound + runningAve)/2; //updates the running average regardless
      }
      //repeats startupCount times to get a running average when the state first begins and leaves the lights off
      else {
        runningAve = (rawSound + runningAve)/2; //calculates the running average
        startupCount = startupCount - 1; //decrements startupCount
        updateLights(0,0); //turns off the lights
      }

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
  
  //delays for a short time, fast enough so the sound can be heard and it can react to it without a visible delay
  delay(5);
}

//outputs a random value for the flicker
int flickerVal() {
  int flickerVal = 255; //sets base rate to 255 (max value)
  flickerVal = flickerVal - random(160); //subtracts a random number from 1 - 160 (gives a nice contrast at this range)
  return flickerVal; //outputs the flicker value
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
    analogWrite(led1, 0);
  }
  
  if (L2_val < 256 && L2_val >= 0) {
    analogWrite(led2, L2_val);
  }
  else {
    analogWrite(led2, 0);
  }
}
