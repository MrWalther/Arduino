#include <Wire.h>
#include "Adafruit_MPR121.h"

Adafruit_MPR121 cap = Adafruit_MPR121();
long lastChange;
int interval;
int state;
bool doBlink;
bool isTouched;
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

void setup() {
  while (!Serial);        // needed to keep leonardo/micro from starting too fast!

  Serial.begin(115200);
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test");

  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");

  delay(1000);
  myBegin();
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  lastChange = millis();
  interval = 2000;
  state = 0;
  doBlink = false;
  isTouched = false;
}

void loop() {
  String temp = "[";
  for (uint8_t i = 0; i < 12; i++) {
    temp += "{\"id\":";
    temp += i;
    temp += ",\"fd\":";
    temp += cap.filteredData(i);
    temp += ",\"bd\":";
    temp += cap.baselineData(i);
    if (i == 11) {
      temp += "}";
    } else {
      temp += "},";
    }
  }
  temp += "]\n";
  Serial.print(temp);

  currtouched = cap.touched();

  for (uint8_t i = 0; i < 12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      if (i == 5 && !doBlink) {
        doBlink = true;
      }
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      //      Serial.print(i); Serial.println(" released");
    }
  }

  // reset our state
  lasttouched = currtouched;

  if (doBlink) {
    blink();
  }

  delay(10);
}

void blink() {
  if (millis() - lastChange >= interval) {
    lastChange = millis();

    switch (state) {
      case 0:
        //        Serial.println("Turning on");
        pinMode(10, OUTPUT);
        pinMode(11, OUTPUT);
        analogWrite(10, 0);
        analogWrite(11, (255 / 5) * 3);
        state = 1;
        break;
      case 2:
        //        Serial.println("Disabling pins");
        analogWrite(10, 0);
        analogWrite(11, 0);
        pinMode(10, INPUT);
        pinMode(11, INPUT);

        //        delay(1000);
        myBegin();
        doBlink = false;
        state = 0;
        break;
    }

  }


  //  if (millis() - lastChange >= interval) {
  //    lastChange = millis();
  //
  //    fieldIsOn = true;
  //    Serial.println("Turning on");
  //
  //
  //    //    if (millis() - lastChange >=
  //    //
  //    //
  //    //        delay(powerOnInterval);
  //    //        delay(powerOffInterval);
  //    //  }
  //  }
}


void myBegin() {
  cap.writeRegister(MPR121_SOFTRESET, 0x63);
  delay(1);
  cap.writeRegister(MPR121_ECR, 0x0);

  uint8_t c = cap.readRegister8(MPR121_CONFIG2);

  if (c != 0x24) return false;


  cap.setThreshholds(8, 4);
  cap.writeRegister(MPR121_MHDR, 0x01);
  cap.writeRegister(MPR121_NHDR, 0x01);
  cap.writeRegister(MPR121_NCLR, 0x0E);
  cap.writeRegister(MPR121_FDLR, 0x00);

  cap.writeRegister(MPR121_MHDF, 0x01);
  cap.writeRegister(MPR121_NHDF, 0x05);
  cap.writeRegister(MPR121_NCLF, 0x01);
  cap.writeRegister(MPR121_FDLF, 0x00);

  cap.writeRegister(MPR121_NHDT, 0x00);
  cap.writeRegister(MPR121_NCLT, 0x00);
  cap.writeRegister(MPR121_FDLT, 0x00);

  cap.writeRegister(MPR121_DEBOUNCE, 0);
  cap.writeRegister(MPR121_CONFIG1, 0x10); // default, 16uA charge current
  cap.writeRegister(MPR121_CONFIG2, 0x20); // 0.5uS encoding, 1ms period

  //  writeRegister(MPR121_AUTOCONFIG0, 0x8F);

  //  writeRegister(MPR121_UPLIMIT, 150);
  //  writeRegister(MPR121_TARGETLIMIT, 100); // should be ~400 (100 shifted)
  //  writeRegister(MPR121_LOWLIMIT, 50);
  // enable all electrodes
  cap.writeRegister(MPR121_ECR, 0x8F);  // start with first 5 bits of baseline tracking
}

