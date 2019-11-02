/*
Space shuttle drive unit firmware

This firmware is optimized for the makerbase MKS GEN L 1.0 board.
author: Shintaro F.
*/

#include <AccelStepper.h>

// pin mapping for MKS Gen L board

// endstop connector
#define POWERHOLD 42  // D42
#define POWERLED  40  // D40
#define LIGHT     34  // A11
#define FAN       9   // D9
#define E1        7   // D7 aka E1
#define E0        10  // D10
#define BED       8   // D8

#define BATTVOLTAGE   A9 // A9

// FR stepper driver x
#define STEPPER1_DIR_PIN    A1
#define STEPPER1_STEP_PIN   A0
#define STEPPER1_STEP_EN    38
// FL stepper driver Y
#define STEPPER2_DIR_PIN    A7
#define STEPPER2_STEP_PIN   A6
#define STEPPER2_STEP_EN    A2
// RR stepper driver Z
#define STEPPER3_DIR_PIN    48
#define STEPPER3_STEP_PIN   46
#define STEPPER3_STEP_EN    A8
// RL stepper driver E0
#define STEPPER4_DIR_PIN    28
#define STEPPER4_STEP_PIN   26
#define STEPPER4_STEP_EN    24


// Define some steppers and the pins the will use
AccelStepper stepper1(AccelStepper::DRIVER, STEPPER1_STEP_PIN, STEPPER1_DIR_PIN);
AccelStepper stepper2(AccelStepper::DRIVER, STEPPER2_STEP_PIN, STEPPER2_DIR_PIN);
AccelStepper stepper3(AccelStepper::DRIVER, STEPPER3_STEP_PIN, STEPPER3_DIR_PIN);
AccelStepper stepper4(AccelStepper::DRIVER, STEPPER4_STEP_PIN, STEPPER4_DIR_PIN);

long lf = 0;
long rf = 0;
long lr = 0;
long rr = 0;

long act_x = 0;
long act_y = 0;
long act_rotate = 0;

long maxRotSpeed = 1000L;  // 16000L
long maxMoveSpeed = 1000L; // 16000L
long moveSpeed = 40L; // 2000L

int millivoltAvg;
int millivolt;
int loopCnt = 0;

#define DTIMEOUT 2000
uint16_t driveTimeout = 0;

void setup()
{
  pinMode(STEPPER1_STEP_EN, OUTPUT); // enable pin LOW = on
  pinMode(STEPPER2_STEP_EN, OUTPUT); // enable pin LOW = on
  pinMode(STEPPER3_STEP_EN, OUTPUT); // enable pin LOW = on
  pinMode(STEPPER4_STEP_EN, OUTPUT); // enable pin LOW = on

  pinMode(BATTVOLTAGE, INPUT);

  pinMode(POWERHOLD,  OUTPUT);
  pinMode(POWERLED,   OUTPUT);
  pinMode(LIGHT,      OUTPUT);
  pinMode(FAN,        OUTPUT);
  pinMode(E1,         OUTPUT);
  pinMode(E0,         OUTPUT);
  pinMode(BED,        OUTPUT);

  digitalWrite(POWERHOLD, HIGH);
  digitalWrite(POWERLED,  HIGH);

  digitalWrite(LIGHT,     LOW);
  digitalWrite(FAN,       LOW);
  digitalWrite(E1,       HIGH);


  analogWrite(FAN,0);

  digitalWrite(E0,    HIGH);
  digitalWrite(BED,   HIGH);

  digitalWrite(STEPPER1_STEP_EN, HIGH); // enable pin LOW = on
  digitalWrite(STEPPER2_STEP_EN, HIGH); // enable pin LOW = on
  digitalWrite(STEPPER3_STEP_EN, HIGH); // enable pin LOW = on
  digitalWrite(STEPPER4_STEP_EN, HIGH); // enable pin LOW = on


  stepper1.setMaxSpeed(maxMoveSpeed);
  // stepper1.setAcceleration(1000);

  stepper2.setMaxSpeed(maxMoveSpeed);
  // stepper2.setAcceleration(1000);

  stepper3.setMaxSpeed(maxMoveSpeed);
  // stepper3.setAcceleration(1000);

  stepper4.setMaxSpeed(maxMoveSpeed);
  // stepper4.setAcceleration(1000);

  Serial.begin(38400);
}

void loop() {
  serialParser();
  stepper1.runSpeed();
  stepper2.runSpeed();
  stepper3.runSpeed();
  stepper4.runSpeed();
  if(((uint16_t)millis() - driveTimeout) > DTIMEOUT) {
    driveTimeout = millis();
    drive(0,0,0);
  }

  if(loopCnt++ > 5000) {
    loopCnt = 0;
    millivoltAvg = millivoltAvg - (millivoltAvg/16) + analogRead(BATTVOLTAGE);
    // 24V = 12064, 12V = 6016
    millivolt = (int)(((long)millivoltAvg * 24000L) / 12160L);
    if(millivolt < 7000) {
      //if(millivolt < 6400) {
      //  Serial.println(F("VBat LOW SHUTDOWN!"));
      //  delay(1000);
      //  pinMode(POWERHOLD, INPUT);
      //}
    }
  }
}




void updateWheels() {
  // calculate wheel speeds
  lf = act_x + act_y + act_rotate;
  rf = act_x - act_y + act_rotate;
  lr = -act_x + act_y + act_rotate;
  rr = -act_x - act_y + act_rotate;

  if((lf == 0) && (rf == 0) && (lr == 0) && (rr == 0)) {
    digitalWrite(STEPPER1_STEP_EN, HIGH); // enable pin LOW = on
    digitalWrite(STEPPER2_STEP_EN, HIGH); // enable pin LOW = on
    digitalWrite(STEPPER3_STEP_EN, HIGH); // enable pin LOW = on
    digitalWrite(STEPPER4_STEP_EN, HIGH); // enable pin LOW = on
  } else {
    digitalWrite(STEPPER1_STEP_EN, LOW); // enable pin LOW = on
    digitalWrite(STEPPER2_STEP_EN, LOW); // enable pin LOW = on
    digitalWrite(STEPPER3_STEP_EN, LOW); // enable pin LOW = on
    digitalWrite(STEPPER4_STEP_EN, LOW); // enable pin LOW = on
  }
  stepper1.setSpeed(-rf);
  stepper2.setSpeed(-lf);
  stepper3.setSpeed(lr);
  stepper4.setSpeed(rr);

}

void drive(long x, long y, long rotate) {
  // later: add acceleration here
  act_x = constrain(x*32L,-16000L,16000L);
  act_y = constrain(y*32L,-16000L,16000L);
  act_rotate = constrain(rotate*-64L,-16000L,16000L);

  updateWheels();
}

void serialParser() {
  static char cmd[32];
  static uint8_t charCount = 0;

  boolean controlUpdate = false;

  while(Serial.available()) {
    char c;
    //digitalWrite(LEDGN1, !digitalRead(LEDGN1));
    c = Serial.read();

    if((c==8) && (charCount>0)) charCount--; // backspace

    if(c>=32) { // char in num char range then add char to cmd array
      cmd[charCount] = c;
      charCount++;
    }

    if(((c == 0x0a) || (c == 0x0d) || (charCount > 30)) ||
      ((charCount==1 ) && ((c=='q') || (c=='w') ||(c=='e') ||(c=='a') ||
      (c=='s') || (c=='d') || (c==' ')))) { // (c == 0x0d) ||
      cmd[charCount] = 0;
      controlUpdate = false;

      if(charCount >= 1) {
        switch(cmd[0]) {
          case 'q':
            drive(0L,0L,-maxRotSpeed);
            driveTimeout = millis();
            break;
          case 'e':
            drive(0L,0L,maxRotSpeed);
            driveTimeout = millis();
            break;
          case 'w':
            drive(0L,moveSpeed,0L);
            driveTimeout = millis();
            break;
          case 's':
            drive(0L,-moveSpeed,0L);
            driveTimeout = millis();
            break;
          case 'a':
            drive(-moveSpeed,0L,0L);
            driveTimeout = millis();
            break;
          case 'd':
            drive(moveSpeed,0L,0L);
            driveTimeout = millis();
            break;

          case 'm': // move
            {
              short xx,yy,rr;
              if(sscanf((const char*)&cmd[1],"%d %d %d",&xx,&yy,&rr)==3) {
                drive(xx,yy,rr);
                Serial.print(act_x);
                Serial.print(',');
                Serial.print(act_y);
                Serial.print(',');
                Serial.println(act_rotate);
                driveTimeout = millis();
              }
            }
            break;

          case ' ': // stop
            // do something
            drive(0,0,0);
            break;

          case 'v':
						Serial.print("batt:");
            Serial.println(millivolt);
            break;

          case 'p':
            if((charCount>=2) && (cmd[1] == '!') ){
                int posi = 900;
                int r;
                // off after 1 minute
                  r = sscanf_P(&cmd[2],PSTR("%i"),&posi);
                  if((r==1) && (posi == 999))
                  {
                    Serial.println("Off after 60s!");
                      drive(0,0,0);
                      for(int i=0; i<60; i++) {
                        delay(1000);
                        digitalWrite(POWERLED, !digitalRead(POWERLED));
                      }
                      digitalWrite(POWERLED, LOW);
                      digitalWrite(POWERHOLD, LOW);
                      //pinMode(POWERHOLD, INPUT);
                  }

              }

            break; // case 'p'

          case 'c':
            if(charCount>=4) {
              int red,green,blue,r;
              r = sscanf_P(&cmd[1],PSTR("%i %i %i"),&red,&green,&blue);
              if(r==3) {
                //if(red>10) {
                  analogWrite(LIGHT, red);
                //} else {
                //  digitalWrite(LIGHT, LOW);
                //}
              }
            }
            break; // case 'c'

        }

        /*
        if(bufcnt >= 4) {
        	uint16_t var_temp;
        	var_temp = (get2Hex((char*)buf))*256U+get2Hex((char*)(buf+2));
        	speed2DistPerSec = (var_temp<0x7fffU) ? var_temp : 0x7fffU;
        }
        */
      }
      charCount = 0;
      Serial.println(">");
    }
  }
}
