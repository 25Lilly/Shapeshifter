#include "TinyWireS.h"                  // wrapper class for I2C slave routines

#define I2C_SLAVE_ADDR  0x0F            // i2c slave address 
#define LED_PIN         4              // ATtiny Pin 3
#define PWM_PIN          1              // ATtiny Pin 6

byte byteRcvd = 0;
int angle = 0;
int pwm_set = 500;
int counter = 0;

bool active = false;

void setup() {
  pinMode(LED_PIN, OUTPUT);            // for general DEBUG use
  pinMode(PWM_PIN, OUTPUT);            // for verification
  Blink(LED_PIN, 2);                   // show it's alive
  TinyWireS.begin(I2C_SLAVE_ADDR);      // init I2C Slave mode
  TinyWireS.onRequest(send_Data);       // Set function for read requests
}


void loop() {

  if (TinyWireS.available()) {          // got I2C input!
    byteRcvd = TinyWireS.read();     // get the byte from master
    angle = (int) byteRcvd * 2;         // Calc desired angle
    pwm_set = calc_PWM(angle);          // Calc desired pwm
    active = true;                      // Set servo control to active
  }

  if (active) {
    runPWM(pwm_set);
    counter ++;
    if (counter > 20) {
      active = false; 
      counter = 0; 
    }
  }

}


void Blink(byte led, byte times) { // poor man's display
  for (byte i = 0; i < times; i++) {
    digitalWrite(led, HIGH);
    delay (250);
    digitalWrite(led, LOW);
    delay (175);
  }
}

int calc_PWM (int angle) {
  //Using equation y = 500 + 200/27 x to calculate desired PWM frequency
  //500 = 0 degrees, 2500 = 270 degrees
  int pwm = 500 + 200 * (angle / 27.0);
  return pwm;
}

void runPWM(int pwm) {
  digitalWrite(PWM_PIN, HIGH);
  delayMicroseconds(pwm);
  digitalWrite(PWM_PIN, LOW);
  delay(50);
}

void send_Data() {
  TinyWireS.write(byteRcvd);
}
