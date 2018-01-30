/***************************
 * E-Drip Tower Version 1.4
 * By: Matt Walliser
 * Edited by: Catherine Tran
 ***************************/

#include <Wire.h>

#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#include <Adafruit_MotorShield.h>
#include <utility/Adafruit_MS_PWMServoDriver.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *myMotor = AFMS.getMotor(1);

#define OFF 0x0
#define ON 0x1

#define VOL 0
#define TIME 1
#define BREW 2
#define DONE 3

void setup() {
  AFMS.begin();
  lcd.begin(16, 2);
  lcd.setBacklight(ON);
  lcd.setCursor(0, 0);
  lcd.print("Brew Volume:    ");
  pinMode(10, OUTPUT);
}

char line[17];
byte latch = 0;
byte state = VOL;
int brewVol = 1000;
int brewTime = 60;
unsigned long secCnt;
byte seconds;
byte duty;

unsigned long stepCnt;

void loop() {
  byte buttons = lcd.readButtons();

  switch (state) {
    case VOL:
      snprintf(line, 17, "%u ml            ", brewVol);
      lcd.setCursor(0, 1);
      lcd.print(line);

      if (!buttons) latch = 0;
      if (buttons && !latch) {
        latch = 1;
        if (buttons & BUTTON_UP || buttons & BUTTON_DOWN) {
          state = TIME;
          lcd.setCursor(0, 0);
          lcd.print("Brew Time:      ");
        }
        if (buttons & BUTTON_LEFT) {
          if (brewVol > 100) brewVol -= 100;
        }
        if (buttons & BUTTON_RIGHT) {
          if (brewVol < 12000) brewVol += 100;
        }
        if (buttons & BUTTON_SELECT) {
          state = BREW;
          lcd.setCursor(0, 0);
          lcd.print("Brewing...      ");
        }
      }
      break;
    case TIME:
      snprintf(line, 17, "%u:%.2u            ", brewTime / 60, brewTime % 60);
      lcd.setCursor(0, 1);
      lcd.print(line);

      if (!buttons) latch = 0;
      if (buttons && !latch) {
        latch = 1;
        if (buttons & BUTTON_UP || buttons & BUTTON_DOWN) {
          state = VOL;
          lcd.setCursor(0, 0);
          lcd.print("Brew Volume:    ");
        }
        if (buttons & BUTTON_LEFT) {
          if (brewTime > 120) brewTime -= 15;
        }
        if (buttons & BUTTON_RIGHT) {
          if (brewTime < 1080) brewTime += 15;
        }
        if (buttons & BUTTON_SELECT) {
          state = BREW;
          lcd.setCursor(0, 0);
          lcd.print("Brewing...      ");
        }
      }
      break;
    case BREW:
      if (latch) {
        latch = 0;
        secCnt = millis();
        seconds = 0;
        duty = (byte)(((float)brewVol / (float)brewTime * 12.592) + 36.587);
        duty = constrain(duty, 0, 255);
        //myMotor->setSpeed(duty);
        //myMotor->run(FORWARD);
        //stepCnt = micros();
        tone(10, brewVol);

        snprintf(line, 17, "%u:%.2u:%.2u         ", brewTime / 60, brewTime % 60, seconds);
        lcd.setCursor(0, 1);
        lcd.print(line);
      }

      /*if (micros() - stepCnt >= 10) {
        stepCnt += 10;
        digitalWrite(10, !digitalRead(10));
      }*/
      
      if (millis() - secCnt >= 1000) {
        secCnt += 1000;
        if (brewTime == 0 && seconds == 0) {
          //myMotor->run(RELEASE);
          noTone(10);
          state = DONE;
          lcd.setCursor(0, 0);
          lcd.print("Brew Done.      ");
          lcd.setCursor(0, 1);
          lcd.print("                ");
        }
        else {
          if (seconds == 0) {
            seconds = 59;
            brewTime--;
          }
          else {
            seconds--;
          }
          snprintf(line, 17, "%u:%.2u:%.2u         ", brewTime / 60, brewTime % 60, seconds);
          lcd.setCursor(0, 1);
          lcd.print(line);
        }
      }
      break;
    case DONE:
      if (millis() - secCnt >= 1000) {
        secCnt += 1000;
        if (latch) {
          latch = 0;
          lcd.setBacklight(ON);
        }
        else {
          latch = 1;
          lcd.setBacklight(OFF);
        }
      }
      break;
    default: break;
  }
}
