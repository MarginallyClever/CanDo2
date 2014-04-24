//------------------------------------------------------------------------------
// CANDO2 - line following robot.  Maze solving is an exercise for the reader
// dan@marginallycelver.com 2014-04-22
//------------------------------------------------------------------------------
// Copyright at end of file.
// please see http://www.github.com/MarginallyClever/CanDo for more information.



//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------



// 0,1, or 2.  each produces more output than the one below.
#define VERBOSE      (1)


#define LEFT_CENTER  (89)  // this will be tuned to your individual servo
#define RIGHT_CENTER (89)  // this will be tuned to your individual servo

#define FORWARD_SPEED (5)
#define TURN_SPEED    (6)

// number of sensors
#define NUM_INPUTS    (5)
// sensor input names
#define EYE_FAR_LEFT  (0)
#define EYE_LEFT      (1)
#define EYE_MIDDLE    (2)
#define EYE_RIGHT     (3)
#define EYE_FAR_RIGHT (4)

#define LED_PIN      (13)

// what are we doing right now?
#define MODE_FOLLOW  (1)
#define MODE_EXPLORE (2)
#define MODE_SOLVE   (3)



//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------



#include <Servo.h>



//------------------------------------------------------------------------------
// GLOBALS
//------------------------------------------------------------------------------



Servo left, right;

int inputs[NUM_INPUTS];
long avg_white[NUM_INPUTS];
long avg_black[NUM_INPUTS];
long cutoff[NUM_INPUTS];
int samples;
int mode=0;



//------------------------------------------------------------------------------
// METHODS
//------------------------------------------------------------------------------



void setup() {
  int i;
  
  // open comms
  Serial.begin(57600);
  Serial.println(F("START"));
  pinMode(LED_PIN,OUTPUT);

  // attach servos
  left.attach(6);
  right.attach(5);
  
  steer(0,0);

#if VERBOSE > 0
  Serial.println(F("Move me so I can see only white..."));
#endif
  wait(3000);  // give user a moment to place robot

  sample_white();
  
#if VERBOSE > 0
  Serial.println(F("Move me so I can see only black..."));
#endif
  wait(3000);  // give user a moment to place robot

  sample_black();

  setup_cutoffs();
  
  wait(3000);  // give user a moment to place robot
  
#if VERBOSE > 0
  Serial.println(F("Move me to the start of the maze..."));
#endif
  wait(3000);  // give user a moment to place robot
  
  mode = MODE_FOLLOW;
  // mode = MODE_EXPLORE;
}


//------------------------------------------------------------------------------
void sample_white() {
  int i;

  // give user a moment to place robot on white
  wait(3000);

  digitalWrite(LED_PIN,HIGH);  // turn on light

#if VERBOSE > 0
  Serial.println(F("Sampling white..."));
#endif
  memset(avg_white,0,sizeof(int)*NUM_INPUTS);
  samples=0;
  long start=millis();
  while(millis()-start < 2000) {
    look();
    samples++;
    for(i=0;i<NUM_INPUTS;++i) {
      avg_white[i]+=inputs[i];
    }
    delay(5);
  }
  
  
#if VERBOSE > 1
  Serial.print(samples);
  Serial.println(F(" white samples: "));
#endif
  for(i=0;i<NUM_INPUTS;++i) {
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
void sample_black() {
  int i;

  digitalWrite(LED_PIN,HIGH);  // turn on light

#if VERBOSE > 0
  Serial.println(F("Sampling black..."));
#endif
  memset(avg_black,0,sizeof(int)*NUM_INPUTS);
  samples=0;
  long start=millis();
  while(millis()-start < 2000) {
    look();
    samples++;
    for(i=0;i<NUM_INPUTS;++i) {
      avg_black[i]+=inputs[i];
    }
    delay(5);
  }
  
#if VERBOSE > 1
  Serial.print(samples);
  Serial.println(F(" black samples: "));
#endif
  for(i=0;i<NUM_INPUTS;++i) {
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
void setup_cutoffs() {
  int i;
  
#if VERBOSE > 1
  Serial.println(F("cutoff: "));
#endif
  for(i=0;i<NUM_INPUTS;++i) {
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
  switch(mode) {
  case MODE_FOLLOW:  follow();  break;
  case MODE_EXPLORE:  explore();  break;
  case MODE_SOLVE:  solve();  break;
  }
}


//------------------------------------------------------------------------------
void look() {
  int i;
  for(i=0;i<NUM_INPUTS;++i) {
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
  int a = LEFT_CENTER - forward + turn;
  int b = RIGHT_CENTER + forward + turn;
  
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
  for(i=0;i<NUM_INPUTS;++i) {
    if(eye_sees_black(i)) return 0;  // eye doesn't see white, quit
  }
  
  return 1;
}


//------------------------------------------------------------------------------
char all_eyes_see_black() {
  int i;
  for(i=0;i<NUM_INPUTS;++i) {
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
    forward = 7;

    // ...and steer
    turn = 0;
    if(!eye_sees_white(EYE_LEFT) || !eye_sees_white(EYE_FAR_LEFT)) {
      // left eye sees
      turn+=5;
    }
    if(!eye_sees_white(EYE_RIGHT) || !eye_sees_white(EYE_FAR_RIGHT)) {
      turn-=5;
    }
  }
  
  steer(forward,turn);
}


//------------------------------------------------------------------------------
void explore() {
  float forward=0, turn=0;
  
  look();
  
  if(all_eyes_see_black()) {
    victory();
  }
  // what kind of intersection have we found?
  if(eye_sees_black(EYE_FAR_RIGHT)) {
    if(eye_sees_black(EYE_FAR_LEFT)) {
      // T intersection?
      // @TODO: turn right and continue
    } else {
      // right hand turn
      // @TODO: turn right and continue
    }
  } else if(eye_sees_black(EYE_FAR_LEFT)) {
    // left hand turn
    // @TODO: turn left and continue
  } else if(all_eyes_see_white()) {
    // we have left the track!  back up!
    forward = -4;
    turn = 0;
  } else {
    // follow line
    // go forward...
    forward = 7;

    // ...and steer
    turn = 0;
    if(!eye_sees_white(EYE_LEFT) || !eye_sees_white(EYE_FAR_LEFT)) {
      // left eye sees
      turn+=5;
    }
    if(!eye_sees_white(EYE_RIGHT) || !eye_sees_white(EYE_FAR_RIGHT)) {
      turn-=5;
    }
  }
  
  steer(forward,turn);
}


//------------------------------------------------------------------------------
void solve() {
  float forward=0, turn=0;
  
  look();
  
  if(all_eyes_see_black()) {
    victory();
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
