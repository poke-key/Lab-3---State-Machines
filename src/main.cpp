#include <Arduino.h>
#include "timerISR.h"

#define LED_RED 11
#define LED_BLUE 10
#define LED_GREEN 9
#define SWITCH  5

#define ANALOG_PHOTO  4
#define ANALOG_POTENTIO  5

#define PHOTO_TRESHOLD 25
#define TIMER_PERIOD 1000
#define COUNTDOWN_TIME 9


unsigned int switchStatus =  LOW ;
unsigned long debounceDuration = 45; // millis
unsigned long lastTimeButtonStateChanged = 0;
int ledStatus = LOW ;
int counter;

enum states { INIT, LED_ON, LED_OFF, RGB, COVERED };
states state = INIT ;

//7-segment display pins and implementation (change pin values if necessary):
int a = 8;
int b = 2;
int c = 4;
int d = 12;
int e = 13;
int f = 7;
int g = 6;
int dp = 10;
int LEDS[7] {a, b, c, d, e, f, g};
//Some values for a-g are provided as examples, implement the remaining a-g values corresponding to the comments below:
//1 = Segment ON, 0 = Segment OFF
int nums[17][7] {
  {1, 1, 1, 1, 1, 1, 0}, //0
  {0, 1, 1, 0, 0, 0, 0}, //1
  {1, 1, 0, 1, 1, 0, 1}, //2
  {1, 1, 1, 1, 0, 0, 1}, //3
  {0, 1, 1, 0, 0, 1, 1}, //4 		<- Implement!
  {1, 0, 1, 1, 0, 1, 1}, //5 		<- Implement!
  {1, 0, 1, 1, 1, 1, 1}, //6 		<- Implement!
  {1, 1, 1, 0, 0, 0, 0}, //7 		<- Implement!
  {1, 1, 1, 1, 1, 1, 1}, //8 		<- Implement!
  {1, 1, 1, 1, 0, 1, 1}, //9 		<- Implement!
  {1, 1, 1, 0, 1, 1, 1}, //10 (A)
  {0, 0, 1, 1, 1, 1, 1}, //11 (b)
  {1, 0, 0, 1, 1, 1, 0}, //12 (C)	<- Implement in hex!
  {0, 1, 1, 1, 1, 0, 1}, //13 (d)	<- Implement in hex!
  {1, 0, 0, 1, 1, 1, 1}, //14 (E)	<- Implement in hex!
  {1, 0, 0, 0, 1, 1, 1}, //15 (F)	<- Implement in hex!
  {0, 0, 0, 0, 0, 0, 0} //off
// a  b  c  d  e  f  g
};

//Call this function in your code to output the integer x on the 7-segment display in hex
void outNum(int x) {
  for (int i = 0; i < 7; i++) {
    if (nums[x][i] == 1) {
      digitalWrite(LEDS[i], 1);
    }
    else {
      digitalWrite(LEDS[i], 0);
    }
  }
}

void stateManager() {
  int pInput = 0 ;

  switch (state) {
    case INIT:
      if(digitalRead(SWITCH) == HIGH) {
        state = RGB;
      }
      else if (analogRead(ANALOG_PHOTO) < PHOTO_TRESHOLD) {
        state = COVERED;
      }
      //outNum(16); // clear display
      break;
    case LED_ON:
      if(digitalRead(SWITCH) == LOW) state = RGB;
      break;
    case LED_OFF: 
      if(digitalRead(SWITCH) == HIGH) state = INIT;
      break;
    case RGB:
      if(digitalRead(SWITCH) == HIGH) state = INIT;
      else state = RGB;
      break;
    case COVERED:
      if(analogRead(ANALOG_PHOTO) < PHOTO_TRESHOLD) {
        counter = 0;
        state = COVERED;
      }
      else if (counter >= COUNTDOWN_TIME) { 
        state = INIT;
        counter = 0;
        outNum(16); // clear display
      }
      else if (digitalRead(SWITCH) == HIGH) {
        state = INIT;
        outNum(16); // clear display
      }
      break;
    default:
      state = INIT;
      break;  

  }

  // State Actions
  switch (state) {
      case INIT:
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_BLUE, LOW);
        digitalWrite(LED_GREEN, LOW);
        break;
      case LED_ON:
        digitalWrite(LED_GREEN, HIGH);
        break;
      case LED_OFF:
        digitalWrite(LED_GREEN, LOW);
        break;
      case RGB:
        pInput = map(analogRead(ANALOG_POTENTIO), 0, 1023, 1, 4);
        if (pInput == 1) {
          // Orange 
          digitalWrite(LED_RED, HIGH);
          digitalWrite(LED_BLUE, LOW);
          digitalWrite(LED_GREEN, LOW);
        }
        else if (pInput == 2) {
          // Yellow
          digitalWrite(LED_RED, LOW);
          digitalWrite(LED_BLUE, HIGH);
          digitalWrite(LED_GREEN, LOW);
        }
        else if (pInput == 3) {
          // White
          digitalWrite(LED_RED, LOW);
          digitalWrite(LED_BLUE, LOW);
          digitalWrite(LED_GREEN, HIGH);
        }
        else if (pInput == 4) {
          // Purple
          digitalWrite(LED_RED, HIGH);
          digitalWrite(LED_BLUE, HIGH);
          digitalWrite(LED_GREEN, HIGH); // Using a different color, like yellow
        }

        break;

      case COVERED:
        digitalWrite(LED_RED, HIGH);
        outNum(COUNTDOWN_TIME - counter);
        counter++;
        break;
      default:
        break;
  }
}

void setup() {
 Serial.begin(9600);
  while(!Serial) {
	  ; // wait for serial port to connect. Needed for native USB
  }

  pinMode(SWITCH, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(ANALOG_PHOTO, INPUT);
  pinMode(A0, OUTPUT);
  digitalWrite(A0, LOW);
  
  analogWrite(SWITCH, 100);

  TimerSet(TIMER_PERIOD);
  TimerOn();

  ///Serial.println(analogRead(ANALOG_PHOTO));

  //outNum(15);
}

void loop() {
  //Serial.println(state);
  stateManager();
  while (!TimerFlag) {
    // wait for timer overflow
  }
  TimerFlag = 0; // reset timer flag
}