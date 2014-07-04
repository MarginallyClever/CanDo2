//------------------------------------------------------------------------------
// CANDO2 - line following robot.  Maze solving is an exercise for the reader
// dan@marginallycelver.com 2014-04-22
//------------------------------------------------------------------------------
// Copyright at end of file.
// please see http://www.github.com/MarginallyClever/CanDo for more information.


//------------------------------------------------------------------------------
// DEFINES - giving human-readable names to constants
//------------------------------------------------------------------------------


// Change this to 0,1, or 2.  Each produces more output than the one before.
// All output is in the Arduino Serial Window
#define VERBOSE      (1)


// The value at which each servo does not move.
// This must be tuned to your individual servo.
#define LEFT_NO_MOVE  (89)
#define RIGHT_NO_MOVE (89)

#define FORWARD_SPEED (7)
#define TURN_SPEED    (5)

#define NUM_EYES      (5)
#define EYE_FAR_LEFT  (0)
#define EYE_LEFT      (1)
#define EYE_MIDDLE    (2)
#define EYE_RIGHT     (3)
#define EYE_FAR_RIGHT (4)

#define LEFT_MOTOR_PIN  (6)
#define RIGHT_MOTOR_PIN (7)
#define LED_PIN         (13)

#define WAIT_TIME   (3000)


//------------------------------------------------------------------------------
// INCLUDES - use free code given to us by Arduino ppl
//------------------------------------------------------------------------------


#include <Servo.h>


//------------------------------------------------------------------------------
// GLOBALS - variables used all over the program
//------------------------------------------------------------------------------


Servo left, right;

int inputs[NUM_EYES];
long avg_white[NUM_EYES];
long avg_black[NUM_EYES];
long cutoff[NUM_EYES];


//------------------------------------------------------------------------------
// METHODS
//------------------------------------------------------------------------------


void setup() {
  int i;
  
  // open comms
  Serial.begin(57600);
  Serial.println(F("START"));
  pinMode(LED_PIN,OUTPUT);

  // the lighting conditions are different in every room.
  // when the robot turns on, give the user 3 seconds to 
  // put the robot's eyes over the white surface and study
  // what white looks like in this light for one second.
  // The LED will turn on while it is studying.
  study_white();
  
  // After that, give the user 3 seconds to 
  // put the robot's eyes over the black surface and study
  // what white looks like in this light for one second.
  // The LED will turn on while it is studying.
  study_black();

  // now that the robot knows what white and black looks like,
  // we can explain the difference.  This takes almost no time.
  learn_difference();

  setup_wheels();
  
  // Give the user another 3 seconds to put the robot at
  // the start of the line.
#if VERBOSE > 0
  Serial.println(F("Move me to the start of the line..."));
#endif
  wait(3000);  // give user a moment to place robot
}


//------------------------------------------------------------------------------
void setup_wheels() {
  // attach servos
  left.attach(LEFT_MOTOR_PIN);
  right.attach(RIGHT_MOTOR_PIN);
  
  // make sure we aren't moving.
  steer(0,0);
}


//------------------------------------------------------------------------------
// average the color white that each eye sees during the next 2 seconds.
void study_white() {
  int i;

#if VERBOSE > 0
  Serial.println(F("Move me so I can see only white..."));
#endif
  wait(WAIT_TIME);  // give user a moment to place robot

  digitalWrite(LED_PIN,HIGH);  // turn on light

#if VERBOSE > 0
  Serial.println(F("Sampling white..."));
#endif
  memset(avg_white,0,sizeof(int)*NUM_EYES);
  int samples=0;
  long start=millis();
  while(millis()-start < 2000) {
    look();
    samples++;
    for(i=0;i<NUM_EYES;++i) {
      avg_white[i]+=inputs[i];
    }
    delay(5);
  }
  
  
  // average = sum / number of samples
#if VERBOSE > 1
  Serial.print(samples);
  Serial.println(F(" white samples: "));
#endif
  for(i=0;i<NUM_EYES;++i) {
    avg_white[i] = (float)avg_white[i] / (float)samples;
#if VERBOSE > 1
  Serial.print(avg_white[i]);
  Serial.print('\t');
#endif
  }
#if VERBOSE > 1
  Serial.print('\n');
#endif

  digitalWrite(LED_PIN,LOW);  // turn off light
}



//------------------------------------------------------------------------------
void study_black() {
  int i;

#if VERBOSE > 0
  Serial.println(F("Move me so I can see only black..."));
#endif
  wait(WAIT_TIME);  // give user a moment to place robot

  digitalWrite(LED_PIN,HIGH);  // turn on light

#if VERBOSE > 0
  Serial.println(F("Sampling black..."));
#endif
  memset(avg_black,0,sizeof(int)*NUM_EYES);
  int samples=0;
  long start=millis();
  while(millis()-start < 2000) {
    look();
    samples++;
    for(i=0;i<NUM_EYES;++i) {
      avg_black[i]+=inputs[i];
    }
    delay(5);
  }

  // average = sum / number of samples
#if VERBOSE > 1
  Serial.print(samples);
  Serial.println(F(" black samples: "));
#endif
  for(i=0;i<NUM_EYES;++i) {
    avg_black[i] = (float)avg_black[i] / (float)samples;
#if VERBOSE > 1
  Serial.print(avg_black[i]);
  Serial.print('\t');
#endif
  }
#if VERBOSE > 1
  Serial.print('\n');
#endif

  digitalWrite(LED_PIN,LOW);  // turn off light
}


//------------------------------------------------------------------------------
void learn_difference() {
  int i;
  
#if VERBOSE > 1
  Serial.println(F("cutoff: "));
#endif
  for(i=0;i<NUM_EYES;++i) {
    cutoff[i] = ( avg_white[i] + avg_black[i] ) / 2.0f;
#if VERBOSE > 1
  Serial.print(avg_black[i]);
  Serial.print('\t');
#endif
  }
#if VERBOSE > 1
  Serial.print('\n');
#endif
}


//------------------------------------------------------------------------------
void loop() {
  follow();
}


//------------------------------------------------------------------------------
void look() {
  int i;
  for(i=0;i<NUM_EYES;++i) {
    inputs[i] = analogRead(A1+i);
#if VERBOSE > 2
    Serial.print(inputs[i]);
    Serial.print('\t');
#endif
  }
#if VERBOSE > 2
  Serial.print('\n');
#endif
}


//------------------------------------------------------------------------------
void steer(float forward,float turn) {
  int a = LEFT_NO_MOVE - forward + turn;
  int b = RIGHT_NO_MOVE + forward + turn;
  
  if(a<10) a=10;  if(a>170) a=170;
  if(b<10) b=10;  if(b>170) b=170;
  
  left.write(a);
  right.write(b);
}


//------------------------------------------------------------------------------
void wait(int ms) {
  // give user a chance to place robot on white
  long start=millis();
  while(millis()-start < ms) {
    delay(5);
  }
}


//------------------------------------------------------------------------------
char eye_sees_white(int i) {
  return inputs[i] > cutoff[i];
}


//------------------------------------------------------------------------------
char eye_sees_black(int i) {
  return inputs[i] < cutoff[i];
}


//------------------------------------------------------------------------------
char all_eyes_see_white() {
  int i;
  for(i=0;i<NUM_EYES;++i) {
    if(eye_sees_black(i)) return 0;  // eye doesn't see white, quit
  }
  
  return 1;
}


//------------------------------------------------------------------------------
char all_eyes_see_black() {
  int i;
  for(i=0;i<NUM_EYES;++i) {
    if(eye_sees_white(i)) return 0;  // eye doesn't see black, quit
  }
  
  return 1;
}


//------------------------------------------------------------------------------
void follow() {
  float forward, turn;
  
  look();
  
  /*if(all_eyes_see_white()) {
    // we have left the track!  back up!
    forward = -4;
    turn = 0;
  } elseif(all_eyes_see_black()) {
    // end of line.  stop!
    victory();
  } else*/ {
    // follow line
    // go forward...
    forward = FORWARD_SPEED;

    // ...and steer
    turn = 0;
    if(!eye_sees_white(EYE_LEFT) || !eye_sees_white(EYE_FAR_LEFT)) {
      // left eye sees
      turn+=TURN_SPEED;
    }
    if(!eye_sees_white(EYE_RIGHT) || !eye_sees_white(EYE_FAR_RIGHT)) {
      turn-=TURN_SPEED;
    }
  }
  
  steer(forward,turn);
}


//------------------------------------------------------------------------------
void victory() {
  steer(0,0);  // stop
  
  int state=HIGH;
  
  while(1) {
    wait(100);
    digitalWrite(LED_PIN,state);
    state = ( state == HIGH ) ? LOW : HIGH;
  }
}



/**
* This file is part of CanDo.
*
* CanDo is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CanDo is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Foobar. If not, see <http://www.gnu.org/licenses/>.
*/
