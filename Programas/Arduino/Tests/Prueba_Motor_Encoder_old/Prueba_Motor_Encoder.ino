#include <digitalWriteFast.h>
#define rightMotor1 5
#define rightMotor2 6
#define D1 4
#define D2 7

#define vmax 255

const byte ledPin = 8;
const byte interruptPinA = 3;
const byte interruptPinB = 2;

//volatile byte state = LOW;
long valor = 0;
long valorant = 0;
long valor_obj = 2500;

long valori = 0;
long valoranti = 0;
long valor_obji = 2500;

struct trayectoria {
	byte h;
	byte t1;
	byte t2;
	byte t3;
};




//Specify the links and initial tuning parameters
//morado
float Kp = 2, Ki = 0.00, Kd = 0;
long Error = 0, Integral = 0, Derivative = 0, Previous_error = 0, t_act = 0, t_prev = 0, dt = 0;
long Output;


void setup() {
  pinMode(rightMotor1, OUTPUT);
  pinMode(rightMotor2, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  digitalWrite(D1, LOW);
  digitalWrite(D2, HIGH);

  pinMode(interruptPinA, INPUT);//INPUT_PULLUP);
  pinMode(interruptPinB, INPUT);//INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPinA), Afunction, RISING);
  Serial.begin(115200);

}

void loop() {
 /* Serial.print("D");  
  Serial.println(valor);
  Serial.print("I");
  Serial.println(valori);*/

	Serial.print("D");
	Serial.print(valor);
	Serial.print("      I");
	Serial.print(valori);
	Serial.print("   ");
	Serial.println(analogRead(A6));

  /*Serial.print("--------------->");
Serial.println(Integral);*/
  t_act = millis();
  dt = t_act - t_prev;
  Error = valor_obj - valor;

  Integral = Integral + (Error*dt);
 
  Derivative = (Error - Previous_error) / dt;


  if (Integral > vmax) { Integral = vmax; }
  t_prev = t_act;
  Previous_error = Error;




  Output = Kp * Error + Ki * Integral + Kd * Derivative;

  if (Output > 0) {
    if (Output > vmax) { Output = vmax; }
    analogWrite(rightMotor1, Output);
    analogWrite(rightMotor2, 0);
  }
  else {
    if (Output < -vmax) { Output = -vmax; }
    analogWrite(rightMotor1, 0);
    analogWrite(rightMotor2, abs(Output));
  }




}

void Afunction() {
  if (digitalReadFast(interruptPinB)) { valor = valor + 1; }
  else { valor = valor - 1; };
}

