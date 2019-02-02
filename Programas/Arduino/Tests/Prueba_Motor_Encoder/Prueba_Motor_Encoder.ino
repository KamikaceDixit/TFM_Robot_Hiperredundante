//#include <digitalWriteFast.h>
#include <millis.h> //http://blog.zakkemble.co.uk/millisecond-tracking-library-for-avr/

#define rightMotor1 5
#define rightMotor2 6 //Cuidado que estan al reves para las pruebas
#define D1 4
#define D2 7
#define ENABLE A3
#define FEEDBACK A7

#define vmax 255

const byte ledPin = 8;
const byte interruptPinA = 3;
const byte interruptPinB = 2;


//volatile byte state = LOW;
long valor = 0;
long valorant = 0;
long valor_obj = 2500;
int trayectoria_iniciada = 0;

long long tiempo_inicio = 0;

long valori = 0;
long valoranti = 0;
long valor_obji = 2500;
long posicion = 0;
long vel = 0;

struct Trapecio {
	long h;
	long t1;
	long t2;
	long t3;
};

Trapecio velocidad = { 2000 , 500 , 3000, 5000 };//500ms arranque esta bien

//millis_t now = millis_get();

//Specify the links and initial tuning parameters
//morado
float Kp = 2, Ki = 0.00, Kd = 0, Kv = 0.116;
long Error = 0, Integral = 0, Derivative = 0, Previous_error = 0,  dt = 0;
long long t_act = 0, t_prev = 0;
long Output;


void setup() {
	
	
	pinMode(rightMotor1, OUTPUT);
	pinMode(rightMotor2, OUTPUT);
	pinMode(D1, OUTPUT);
	pinMode(D2, OUTPUT);
	pinMode(ENABLE, OUTPUT);
	pinMode(FEEDBACK, INPUT);
	digitalWrite(ENABLE, HIGH);
	digitalWrite(D1, LOW);
	digitalWrite(D2, HIGH);

	pinMode(interruptPinA, INPUT);//INPUT_PULLUP);
	pinMode(interruptPinB, INPUT);//INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(interruptPinA), Afunction, RISING);
	Serial.begin(115200);

	millis_init();
	setPwmFrequency(5, 8);  //Los pines 5 y 6 tienen una frecuencia base de 62500Hz, normalmente su divisor es 64, por lo que 
							//la frecuencia asociada al PWM es 976Hz. Con esta funcion se varia el divisor a 8, obteniendo
							//asi una frecuencia de 7812Hz para evitar problemas con la conmutacion de los motores.
							//Esto afecta al timer0, que esta asociado a las funciones de tiempo millis() y delay(), por lo 
							//que sera necesario corregirlo en el codigo en caso de utilizarlas.
	
	/*Serial.print("Tiempo,");
	Serial.print("Posicion_teorica,");
	Serial.println("Posicion_Real");*/
}

void loop() {
	
	//Serial.print("I");
	//Serial.println(valori);
	

	if (!trayectoria_iniciada) { delay_mod(1000); tiempo_inicio = millis_get(); trayectoria_iniciada = 1; valor = 0; t_prev = millis_get()- tiempo_inicio; }

	if (trayectoria_iniciada) {
		t_act = millis_get() - tiempo_inicio;
		dt = t_act - t_prev;

		if (t_act - velocidad.t1 < 0)
		{
			vel = t_act * velocidad.h / velocidad.t1;
			posicion = 0.001*(0.5* t_act * t_act*velocidad.h / velocidad.t1); //integral de la velocidad
		}
		else if (((t_act - velocidad.t1) >= 0) && ((t_act - velocidad.t2) <= 0))
		{
			vel = velocidad.h;
			posicion = 0.001*(0.5*velocidad.h*velocidad.t1 + velocidad.h*(t_act - velocidad.t1));
		}
		else if (t_act - velocidad.t3 > 0) { trayectoria_iniciada = 0; Error = 0; Output = 0;  }
		else
		{
			vel = velocidad.h*(velocidad.t3 - t_act) / (velocidad.t3 - velocidad.t2);
			posicion = 0.001*(0.5*velocidad.h*velocidad.t1 +
				velocidad.h*(velocidad.t2 - velocidad.t1) +
				(0.5*velocidad.h*(velocidad.t3 - velocidad.t2) - 0.5*(velocidad.t3 - t_act)*velocidad.h*(velocidad.t3 - t_act) / (velocidad.t3 - velocidad.t2)));
		}

		if (dt >= 10)
		{
			
			Serial.print(posicion);
			Serial.print(" ");
			Serial.print(valor);
			Serial.print(" ");
			Serial.print(Output);
			Serial.print(" ");
			Serial.println(corriente_motor());
			t_prev = t_act;
		}


		Error = posicion - valor;

		Integral = Integral + (Error*dt);

		Derivative = (Error - Previous_error) / dt;


		/*if (Integral > vmax) { Integral = vmax; }
		t_prev = t_act;
		Previous_error = Error;*/




		Output = Kp * Error + Kv * vel;/**/ // Ki * Integral + Kd * Derivative;
		//Serial.print(analogRead(A6));
		//Serial.print("    ");
		
		//Serial.println(Output);
		
		if (Output > 0) {
			if (Output > vmax) { Output = vmax; }
			analogWrite(rightMotor2, Output);
			analogWrite(rightMotor1, 0);
		}
		else analogWrite(rightMotor1, 0);
		/*else {
			if (Output < -vmax) { Output = -vmax; }
			analogWrite(rightMotor1, 0);
			analogWrite(rightMotor2, abs(Output));
		}*/

		




	}

	
}

void Afunction() {
	if (digitalRead(interruptPinB)) { --valor; }
	else { ++valor; };
}

float corriente_motor() {
	return analogRead(FEEDBACK)*(5.0/1024.0)*(375.0 / 200.0);
}

void delay_mod(long tiempo) {
	volatile long long t_init = millis_get();
	while (millis_get() - t_init < tiempo) {}
}

void setPwmFrequency(int pin, int divisor) {
	byte mode;
	if (pin == 5 || pin == 6 || pin == 9 || pin == 10) {
		switch (divisor) {
		case 1: mode = 0x01; break;
		case 8: mode = 0x02; break;
		case 64: mode = 0x03; break;
		case 256: mode = 0x04; break;
		case 1024: mode = 0x05; break;
		default: return;
		}
		if (pin == 5 || pin == 6) {
			TCCR0B = TCCR0B & 0b11111000 | mode;
		}
		else {
			TCCR1B = TCCR1B & 0b11111000 | mode;
		}
	}
	else if (pin == 3 || pin == 11) {
		switch (divisor) {
		case 1: mode = 0x01; break;
		case 8: mode = 0x02; break;
		case 32: mode = 0x03; break;
		case 64: mode = 0x04; break;
		case 128: mode = 0x05; break;
		case 256: mode = 0x06; break;
		case 1024: mode = 0x07; break;
		default: return;
		}
		TCCR2B = TCCR2B & 0b11111000 | mode;
	}
}


