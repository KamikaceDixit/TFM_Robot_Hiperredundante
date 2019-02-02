const byte ledPin = 13;
const byte interruptPinA = 2;
const byte interruptPinB = 3;
volatile byte state = LOW;
double valor=0;
double valorant=0;
#include <Wire.h>

typedef union
{
 double number;
 uint8_t bytes[4];
} doubleUNION_t;
doubleUNION_t valord;


void setup() {
 // pinMode(10, OUTPUT);
  //pinMode(11, OUTPUT);
  pinMode(interruptPinA, INPUT);//INPUT_PULLUP);
  pinMode(interruptPinB, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPinA), Afunction, RISING);
  //attachInterrupt(digitalPinToInterrupt(interruptPinB), Bfunction, RISING);
  Serial.begin(9600);
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event

}

void loop() {
  valord.number=valor;
}

void requestEvent() {
  Wire.write(valord.bytes,4); // respond with message of 4 bytes
  // as expected by master
}
void Afunction() {
  if(digitalRead(interruptPinB)){valor=valor+1;}
  else{valor=valor-1;};
}

