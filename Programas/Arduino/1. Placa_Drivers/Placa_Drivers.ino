#include <SPI.h>
#include <mcp2515.h>
#include <millis.h> //Utiliza el timer 2
#include <digitalWriteFast.h>
#include <SingleEMAFilterLib.h>



#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>  //44 pulsos por vuelta


//Parametros para la identificacion de placa Driver Unica
#define DriverID 32
#define IDs_per_Driver 50
#define Vel_Samples 5
#define Vel_Sample_Time 5 //ms
#define Dead_Input 20 //Rango PWM a partir del cual tiene efecto en el motor. Aprox 30, default:20
#define Tiempo_Gracia 0.05 //Tiempo que se le deja para que termine de alcanzar la posicion (s)
#define CAN_Tr_Data_Space 8 // Direcciones desde la posicion 1 de la trayectoria hasta la siguiente ej: 0 - 7

#define Max_Puntos_Por_Trayectoria 2

//Opciones Debugger
#define Serial_Debugger 0
#define Serial_Debugger2 0

//Definicion Pines de entrada
#define FS A6
#define FB A7
#define SOS A0
#define End_Stop A1
#define Aux_Input A2
#define ENABLE A3
#define ENC1 2
#define ENC2 3
#define D1 4
#define DRIVER1 5
#define DRIVER2 6
#define D2 7		//Esta negado en el driver
#define LED1 8
#define INT 9
#define C_CS 10


//Definicion Direcciones Bus-CAN utilizadas
	//Direccion del estado de los finales de carrera
#define FDC_ID 2000
#define FDC_LENGTH 5
	//Direccion para peticion de estado finales de carrera
#define UPDATE_REQ_ID 2001
#define UPDATE_REQ_LENGTH 1

//Direcciones Bus-CAN Propias de la placa
#define CAN_Tr_Pos_ID 0
#define CAN_Tr_Pos_Len 4

#define CAN_Tr_Vel_ID 1
#define CAN_Tr_Vel_Len 4

#define CAN_Tr_Acelt_ID 2
#define CAN_Tr_Acelt_Len 4

#define CAN_Tr_Dcelt_ID 3
#define CAN_Tr_Dcelt_Len 4

#define CAN_Tr_Vel_Max_ID 4
#define CAN_Tr_Vel_Max_Len 4

#define CAN_Tr_Time_ID 5
#define CAN_Tr_Time_Len 4

#define CAN_Corriente_ID 41
#define CAN_Corriente_Len 5

#define CAN_Dato_Recv_ID 42
#define CAN_Dato_Recv_Len 1

#define CAN_Pos_Alc_ID 43
#define CAN_Pos_Alc_Len 1

#define CAN_Tr_Env_ID 44
#define CAN_Tr_Env_Len 1

#define CAN_Tr_Recv_ID 45
#define CAN_Tr_Recv_Len 1

#define CAN_Ini_Tr_ID 46
#define CAN_Ini_Tr_Len 1

#define CAN_LED_ID 47
#define CAN_LED_Len 1

#define CAN_Pos_Mot_ID 49
#define CAN_Pos_Mot_Len 4



typedef union   //Guarda el valor en cadena de bytes y luego lo puedo leer en float
{
	float Valor;
	byte Cadena[4];
} FloatUnion;

typedef union
{
	long Valor;
	byte Cadena[4];
} LongUnion;

typedef union
{
	boolean Valor;
	byte Cadena[1];
} BoolUnion;


can_frame ReadMsg;		//Mensaje Reservado para lectura
can_frame WriteMsg;
can_frame UpdateReqFdC;

Encoder Motor_Enc(ENC2, ENC1);

MCP2515 mcp2515(C_CS);   //Pin comunicación SPI Arduino-MCP2515
byte state_FdC[5] = { 0 };
boolean own_FdC = false; //Estado del FdC asociado a la placa

//Definición variable Motor
struct MotorData
{
	long long Pulses_Ant;
	long long t_prev_vel;
	float Vel[Vel_Samples];
	float Vel_F;
};

struct FeedForward
{
	float Input, Output, Kv, Ka;
};

struct PointsData
{
	//Vamos a ver pollito, por BusCAN vamos a recibir solamente Posiciones y Tiempos para esas posiciones
	int Points = 2;
	float MaxSpeed[Max_Puntos_Por_Trayectoria - 1];
	float AcelTime[Max_Puntos_Por_Trayectoria - 1];
	float DcelTime[Max_Puntos_Por_Trayectoria - 1];
	float a, b, c, d, e;
	float Time[Max_Puntos_Por_Trayectoria];
	float Pulses[Max_Puntos_Por_Trayectoria];
	float Speed[Max_Puntos_Por_Trayectoria];
	float Acel[Max_Puntos_Por_Trayectoria];
	float Time_Ini = 0;
};

//Definición variable PIDF
struct PIDData
{
	float Kp, Ki, Kd, Kb, Kn;
	int Sample_time;
	float Error, Proportional, Integral, Derivative, Feedback, Previous_error, Error_WindUp, Derivative_Sum;
	float min_limit, max_limit;
	long long t_act, t_prev;
	long int dt;
	float Input, Output;
	//Para desabilitar el filtro anti Wind-up, igualar Kb a 0 o los limites a 0
};


//Variables Motor
MotorData Motor = { 0 };

//Variables para los PIDs
PIDData PIDVel = { 0 };
PIDData PIDPos = { 0 };
FeedForward FF_1 = { 0 };

//Variables Trayectoria
PointsData Tr_P;	//Definicion de la trayectoria a partir de los puntos recibidos por Bus CAN
					//Para las trayectorias, en pulsos y milisegundos pls
PointsData Tr_1;
PointsData Tr_2;
PointsData Tr_3;


SingleEMAFilter<float> EMAFilter(0.5); //Parametro alfa del filtro
int CAN_Dirs[50] = { 0 };

void setup() {
	//Definicion de los pines de entrada
	//Los encoder se definen dentro de la libreria
	pinMode(FS, INPUT);
	pinMode(FB, INPUT);
	pinMode(SOS, INPUT);
	pinMode(End_Stop, INPUT);
	pinMode(Aux_Input, INPUT);
	pinMode(ENABLE, OUTPUT);
	pinMode(LED1, OUTPUT);
	pinMode(D1, OUTPUT);
	pinMode(DRIVER1, OUTPUT);
	pinMode(DRIVER2, OUTPUT);
	pinMode(D2, OUTPUT);			//Esta negado en el driver
	pinMode(INT, OUTPUT);
	pinMode(C_CS, OUTPUT);

	digitalWrite(ENABLE, LOW);

	TCCR0B = TCCR0B & 0b11111000 | 0x02;	//Los pines 5 y 6 tienen una frecuencia base de 62500Hz, normalmente su divisor es 64, por lo que 
											//la frecuencia asociada al PWM es 976Hz. Con esta funcion se varia el divisor a 8, obteniendo
											//asi una frecuencia de 7812Hz para evitar problemas con la conmutacion de los motores.
											//Esto afecta al timer0, que esta asociado a las funciones de tiempo millis() y delay(), por lo 
											//que sera necesario corregirlo en el codigo en caso de utilizarlas.

	millis_init();			//Inicia la funcion millis alternativa debido a la modificacion del timer0.

	CAN_Dirs_init();
	//Definicion mensajes CAN
	UpdateReqFdC.can_id = UPDATE_REQ_ID;
	UpdateReqFdC.can_dlc = UPDATE_REQ_LENGTH;


	SPI.begin();	//Inicio SPI para comunicacion con el chip Bus-CAN

	while (!Serial);
	Serial.begin(115200);

	mcp2515.reset();
	mcp2515.setBitrate(CAN_125KBPS);
	mcp2515.setNormalMode();
	//mcp2515.setLoopbackMode();

	//Variables Prealimentacion
	FF_1.Ka = 0;
	FF_1.Kv = 1;

	//Variables del PID de velocidad
	PIDVel.Kp = 0.041367;
	PIDVel.Ki = 1.084;			//1.06885687507539;
	PIDVel.Kd = 0.0001;// 0.000074;		//0.055;
	PIDVel.Kb = 1;				//0.108;
	PIDVel.Kn = 76;
	PIDVel.Sample_time = 5;		//ms
	PIDVel.max_limit = 255;
	PIDVel.min_limit = -255;
	PIDVel.Input = 0;			//pulsos/s

	//Variables del PID de Posicion
	PIDPos.Kp = 5.95;// 5.746;
	PIDPos.Ki = 0;//0.65;
	PIDPos.Kd = 0;//0.0625;
	PIDPos.Kn = 100;
	PIDPos.Sample_time = 5;		//ms

	reset_Tr(&Tr_P);

	//Definicion de la trayectoria

	/*
	Tr_P.AcelTime[0] = 0.5;
	Tr_P.DcelTime[0] = 0.5;

	Tr_P.MaxSpeed[0] = 0;
	Tr_P.Time[0] = 0;
	Tr_P.Time[1] = 2;
	Tr_P.Pulses[0] = 0;
	Tr_P.Pulses[1] = 27500; //No sirve para nada, los pulsos se consiguen a partir de la integral.
	Tr_P.Speed[0] = 0;
	Tr_P.Speed[1] = 0;
	Gen_Tr(&Tr_P);

	*/


	/*
	for (float i = Tr_P.Time[0]; i <= Tr_P.Time[1]; i += 0.01) {
		float temp = Tr_Pos(i);
		Serial.print(i);
		for (float j = 0; j <= temp; j += 40) {
			Serial.print("-");
		}
		Serial.print("   ");
		Serial.println(temp);
	}*/



}


int velo = 0;
int incomingByte = 0;   // for incoming serial data
boolean data_logging = false;
int dt = 0; //Diferencia de tiempos con el ultimo muestreo.
int Trayectoria_Iniciada = 0; //Una vez que comienza la trayectoria se pone a 1.
BoolUnion AuxB = { 0 };

void loop() {

	if (Trayectoria_Iniciada) {
		//Calcula la velocidad del motor en funcion de los pulsos de encoder cada Periodo Vel_Sample_Time
		dt = millis_get() - Motor.t_prev_vel;
		if (dt >= Vel_Sample_Time) {
			eval_vel(dt); //Calcula la velocidad de giro a partir de la diferencia de pulsos de encoder.
		}
		//Evalua el PID de posicion cada periodo PIDPos.Sample_time
		PIDPos.dt = millis_get() - PIDPos.t_prev;
		if (PIDPos.dt >= PIDPos.Sample_time) {
			PIDPos.Feedback = Motor_Enc.read();
			PIDPos.Input = Tr_Pos((seconds_get() - Tr_P.Time_Ini)) - PIDPos.Feedback;
			eval_PIDF(&PIDPos);

			Serial_Trayectoria(); //Para imprimir la trayectoria por serial
		}
		//Evalua el PID de velocidad cada Periodo PIDVel.Sample_time
		PIDVel.dt = millis_get() - PIDVel.t_prev;
		if (PIDVel.dt >= PIDVel.Sample_time) {
			eval_FF(&FF_1);//Prealimentacion
			PIDVel.Feedback = Motor.Vel_F;
			PIDVel.Input = PIDPos.Output - PIDVel.Feedback + FF_1.Output; //Entrada al bloque sumador
			eval_PIDF(&PIDVel);
			//Si la salida se encuentra en la zona muerta del motor, se deja en rotacion libre ya que no
			//tendrá ningún efecto.
			if (PIDVel.Output > Dead_Input) { FwMotor((int)PIDVel.Output); }
			else if (PIDVel.Output < -Dead_Input) { RvMotor((int)abs(PIDVel.Output)); }
			else { digitalWrite(ENABLE, LOW); FwMotor(0); }
		}

		if ((seconds_get() - Tr_P.Time_Ini) > (Tr_P.Time[1] + Tiempo_Gracia)) {
			Parar_Tr();// Una vez pasado el tiempo teorico de la trayectoria, se puede dejar un tiempo posterior para que el
			//PID intente llegar a la posicion final en el caso que no la haya alcanzado todavia. Una vez pasado ese tiempo,
			//se detiene la trayectoria.
		}
	}



	//Escucha la direccion CAN de peticiones de estado de los FdC
	if (mcp2515.readMessage(&ReadMsg) == MCP2515::ERROR_OK) {
		if ((ReadMsg.can_id >= CAN_Dirs[0]) && (ReadMsg.can_id <= CAN_Dirs[40])) {//Datos de trayectoria
			Serial.println(ReadMsg.can_id);
			Recibir_Tr();

		}
		else if (ReadMsg.can_id == FDC_ID) { save_FdC(); } //Guarda el estado de FdC en OwnFdC cuando se envia una actualizacion
		else if (ReadMsg.can_id == 1800) {
			Serial.println("Comenzar"); 
			Comenzar_Tr();
		}


		else if (ReadMsg.can_id >= CAN_Dirs[41] && ReadMsg.can_id <= CAN_Dirs[49]) {//Datos Auxiliares
			FloatUnion Aux = { 0 };
			int ID_Aux = ReadMsg.can_id - CAN_Dirs[0]; //Referencia a 0 el ID

			switch (ID_Aux) {
			case CAN_Corriente_ID:
				if (ReadMsg.can_dlc == 1) {
					Aux.Valor = corriente_motor();
					WriteMsg.can_dlc = CAN_Corriente_Len;
					WriteMsg.can_id = (DriverID - 1) * IDs_per_Driver + CAN_Corriente_ID;
					memcpy(WriteMsg.data, Aux.Cadena, WriteMsg.can_dlc);
					while (mcp2515.sendMessage(&WriteMsg)!=0);
				}
				break;
			case CAN_Dato_Recv_ID:
				break;
			case CAN_Pos_Alc_ID:
				break;
			case CAN_Tr_Env_ID:
				//Si me ha terminado de enviar los datos de la trayectoria, la genero.
				Serial.print("Time0: "); Serial.println(Tr_P.Time[0]);
				Serial.print("Time1: "); Serial.println(Tr_P.Time[1]);
				Serial.print("AcelT: "); Serial.println(Tr_P.AcelTime[0]);
				Serial.print("DcelT: "); Serial.println(Tr_P.DcelTime[0]);
				Serial.print("MaxSpeed: "); Serial.println(Tr_P.MaxSpeed[0]);

				Gen_Tr(&Tr_P);

				//Envia el mensaje de que ha recibido la trayectoria :)

				AuxB.Valor = true;
				WriteMsg.can_dlc = 1;
				memcpy(WriteMsg.data, AuxB.Cadena, WriteMsg.can_dlc);
				WriteMsg.can_id = CAN_Tr_Recv_ID;
				while (mcp2515.sendMessage(&WriteMsg)!=0);
				//Hay que comprobar que ha enviado todo y una vez comprobado le decimos que 
				//lo hemos recibido y estamos listos para comenzar

				break;
			case CAN_Tr_Recv_ID:
				break;
			case CAN_Pos_Mot_ID:
				if (ReadMsg.can_dlc == 1) {
					CAN_Update_Pos();
				}
				break;

			case CAN_LED_ID:
				memcpy(AuxB.Cadena, WriteMsg.data, WriteMsg.can_dlc);
				digitalWrite(LED1, AuxB.Valor);
				break;


			default:
				break;
			}
		}
	}

	if (Trayectoria_Iniciada && own_FdC && (Tr_1.Speed[1]<=0)) { //Solo se mueva en direccion contraria al FdC
		Parar_Tr();
	}
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------

void CAN_Update_Pos() {
	FloatUnion AuxF = { 0 };
	AuxF.Valor = Motor_Enc.read();
	WriteMsg.can_dlc = CAN_Pos_Mot_Len;
	WriteMsg.can_id = (DriverID - 1) * IDs_per_Driver + CAN_Pos_Mot_ID;
	memcpy(WriteMsg.data, AuxF.Cadena, WriteMsg.can_dlc);
	while(mcp2515.sendMessage(&WriteMsg) != 0);
}
void CAN_Dirs_init() {
	for (int i = 0; i < IDs_per_Driver; i++) {
		CAN_Dirs[i] = (DriverID - 1) * IDs_per_Driver + i;
	}

}
void Comenzar_Tr() {
	if (Trayectoria_Iniciada == 0) {
		reset_PIDF(&PIDPos);
		reset_PIDF(&PIDVel);
		Tr_P.Time_Ini = seconds_get();
		Motor.t_prev_vel = millis_get();
		Motor.Vel_F = 0;
		Motor.Pulses_Ant = Motor_Enc.read();
		Trayectoria_Iniciada = 1;

	}
}

float reset_Tr(struct PointsData *Tr) {
	//Definicion de la trayectoria
	Tr->AcelTime[0] = 0.5;
	Tr->DcelTime[0] = 0.5;
	Tr->MaxSpeed[0] = 0;
	Tr->Time[0] = 0;
	Tr->Time[1] = 0;
	Tr->Pulses[0] = 0; //Motor_Enc.read();
	Tr->Pulses[1] = 0; //No sirve para nada, los pulsos se consiguen a partir de la integral.
	Tr->Speed[0] = 0;
	Tr->Speed[1] = 0;
}

void Parar_Tr() {
	Trayectoria_Iniciada = 0;
	BrakeMotor();
	Serial.println("Parado");
	EMAFilter.ResetValue();
	/*reset_Tr(&Tr_P);
	reset_Tr(&Tr_1);
	reset_Tr(&Tr_2);
	reset_Tr(&Tr_3);*/
}
void Serial_Trayectoria() {
	//Tiempo
	Serial.print((seconds_get() - Tr_P.Time_Ini), 3);
	Serial.print(",\t");
	//Trayectoria Posicion
	Serial.print(Tr_Pos(seconds_get() - Tr_P.Time_Ini));
	Serial.print(",\t");
	//Posicion Real
	Serial.print(Motor_Enc.read());
	Serial.print(",\t");
	//Trayectoria Velocidad
	Serial.print(Tr_Vel(seconds_get() - Tr_P.Time_Ini));
	//Serial.print((int)PIDVel.Output);
	Serial.print(",\t");
	//Trayectoria Velocidad
	Serial.print(Motor.Vel_F);
	Serial.print(",\t");
	//Velocidad
	//Serial.println(Motor.Vel_F);
	Serial.println(PIDVel.Output);
}

void FwMotor(int vel) {
	digitalWrite(ENABLE, HIGH);
	digitalWrite(D1, LOW);
	digitalWrite(D2, HIGH);
	analogWrite(DRIVER1, vel);
	digitalWrite(DRIVER2, LOW);
}

void RvMotor(int vel) {
	digitalWrite(ENABLE, HIGH);
	digitalWrite(D1, LOW);
	digitalWrite(D2, HIGH);
	analogWrite(DRIVER2, vel);
	digitalWrite(DRIVER1, LOW);
}

void BrakeMotor() {
	digitalWrite(ENABLE, HIGH);
	digitalWrite(D1, LOW);
	digitalWrite(D2, HIGH);
	analogWrite(DRIVER1, LOW);
	digitalWrite(DRIVER2, LOW);
}


void prntBits(int b)
{
	for (int i = 7; i >= 0; i--)
	{
		Serial.print(bitRead(b, i));
	}
	Serial.println("");
}

void save_FdC() {
	memcpy(state_FdC, ReadMsg.data, ReadMsg.can_dlc); //Copia ReadMsg.data en State_FdC
	//Compara si el FdC pulsado corresponde a esta placa
	if ((state_FdC[(DriverID - 1) / 8] & (0b00000001 << (DriverID - 1) % 8)) != 0)
	{
		own_FdC = true; //Se recibe el mensaje con los estados de los FdC, si el resultado
						//es positivo significa que el FdC de esta placa está pulsado.
	}
	else { own_FdC = false; }
}


void Recibir_Tr() {
	FloatUnion Aux = { 0 };
	memcpy(Aux.Cadena, ReadMsg.data, ReadMsg.can_dlc);
	//Ya sabemos que los ID's son de trayectoria y corresponden a esta placa, 
	//por lo que lo referenciamos a 0 para facilitar el manejo
	int ID_Aux = ReadMsg.can_id - CAN_Dirs[0];

	switch (ID_Aux % CAN_Tr_Data_Space)
	{
	case 0:
		Tr_P.Pulses[ID_Aux / (int)CAN_Tr_Data_Space] = Aux.Valor;
		break;
	case 1:
		Tr_P.Speed[ID_Aux / (int)CAN_Tr_Data_Space] = Aux.Valor;
		break;
	case 2:
		Tr_P.AcelTime[ID_Aux / (int)CAN_Tr_Data_Space] = Aux.Valor;
		break;
	case 3:
		Tr_P.DcelTime[ID_Aux / (int)CAN_Tr_Data_Space] = Aux.Valor;
		break;
	case 4:
		Tr_P.MaxSpeed[ID_Aux / (int)CAN_Tr_Data_Space] = Aux.Valor;
		break;
	case 5:
		Tr_P.Time[ID_Aux / (int)CAN_Tr_Data_Space] = Aux.Valor;
		//Serial.println(ID_Aux / (int)CAN_Tr_Data_Space);
	default:
		break;
	}
	//Definicion de la trayectoria
	/*Tr_P.AcelTime[0] = 0.5;
	Tr_P.DcelTime[0] = 0.5;

	Tr_P.MaxSpeed[0] = 0;
	Tr_P.Time[1] = 1;
	Tr_P.Pulses[0] = 0;
	Tr_P.Pulses[1] = 27500; //No sirve para nada, los pulsos se consiguen a partir de la integral.
	Tr_P.Speed[0] = 0;
	Tr_P.Speed[1] = 0;
	Gen_Tr(&Tr_P);*/
}

float corriente_motor() {
	return analogRead(FB)*(5.0 / 1024.0)*(375.0 / 200.0);	//El Driver MC33887 tiene feedback de corriente,
															//la cual es leida con esta funcion.
}

void delay_mod(long tiempo) {
	volatile long long t_init = millis_get();
	while (millis_get() - t_init < tiempo) {}
}


void eval_vel(float dt) {
	//EMAFilter.AddValue() devuelve directamente el valor filtrado LP.
	//Se multiplica por 1000 para obtener pulsos/s, ya que dt viene dado en ms.
	Motor.Vel_F = EMAFilter.AddValue(1000 * (Motor_Enc.read() - Motor.Pulses_Ant) / (float)dt);
	Motor.Pulses_Ant = Motor_Enc.read();
	Motor.t_prev_vel = millis_get();
}

void eval_FF(struct FeedForward *FF) {
	FF->Output = FF->Kv * Tr_Vel(seconds_get() - Tr_P.Time_Ini) +
		FF->Ka*Tr_Acel(seconds_get() - Tr_P.Time_Ini);
}


void eval_PIDF(struct PIDData *PID) {
	//Cuidadoooooo, hay que mirar que hacer con T_prev, se va a la puta cuando sale del pid
	PID->t_act = millis_get();
	PID->dt = PID->t_act - PID->t_prev;
	PID->Error = PID->Input;
	PID->Proportional = PID->Error*PID->Kp;
	PID->Integral += (PID->Error*PID->Ki + PID->Kb*PID->Error_WindUp)*0.001*PID->dt;

	if (abs(PID->Error - PID->Previous_error) > 0.01) { //Evitar cociente 0/0
		if (PID->Kn > 0) { //Filtro derivativo
			PID->Derivative_Sum += PID->Derivative*0.001*PID->dt;
			PID->Derivative = ((PID->Error*PID->Kd) - PID->Derivative_Sum)*PID->Kn;
		}
		else {
			PID->Derivative = PID->Error * (PID->Error - PID->Previous_error) / (float)(0.001*PID->dt);
		}
	}
	else { PID->Derivative = 0; }

	PID->Output = PID->Proportional + PID->Integral + PID->Derivative;

	PID->t_prev = PID->t_act;
	PID->Previous_error = PID->Error;

	if (PID->max_limit != 0 || PID->min_limit != 0) {
		PID->Error_WindUp = PID->Output;
		if (PID->Output > PID->max_limit) {
			PID->Output = PID->max_limit;
		}
		else if (PID->Output < PID->min_limit) {
			PID->Output = PID->min_limit;
		}
		PID->Error_WindUp = PID->Output - PID->Error_WindUp;  //Filtro WindUp
	}
}

void reset_PIDF(struct PIDData *PID) {
	PID->Error = 0;
	PID->Proportional = 0;
	PID->Integral = 0;
	PID->Derivative = 0;
	PID->Previous_error = 0;
	PID->Error_WindUp = 0;
	PID->Derivative_Sum = 0;
	PID->t_act = 0;
	PID->t_prev = millis_get();
	PID->Output = 0;
	PID->dt = 0;
	PID->Input = 0;
}
float Gen_Tr(struct PointsData *Tr) {
	Tr_1.Time[0] = Tr->Time[0];
	Tr_1.Time[1] = Tr->Time[0] + Tr->AcelTime[0];
	Tr_1.Speed[0] = Tr->Speed[0];
	Tr_1.Speed[1] = Tr->MaxSpeed[0];
	Tr_1.Acel[0] = 0;
	Tr_1.Acel[1] = 0;
	Tr_1.Pulses[0] = Tr->Pulses[0];
	Calcular_Coef(&Tr_1);
	Tr_1.Pulses[1] = Tr_Pos(Tr_1.Time[1]);

	Tr_2.Time[0] = Tr_1.Time[1];
	Tr_2.Time[1] = Tr->Time[1] - Tr->DcelTime[0];
	Tr_2.Speed[0] = Tr->MaxSpeed[0];
	Tr_2.Speed[1] = Tr->MaxSpeed[0];
	Tr_2.Pulses[0] = Tr_1.Pulses[1];
	Calcular_Coef(&Tr_2);
	Tr_2.Pulses[1] = Tr_Pos(Tr_2.Time[1]);

	Tr_3.Points = 2;
	Tr_3.Time[0] = Tr_2.Time[1];
	Tr_3.Time[1] = Tr->Time[1];
	Tr_3.Speed[0] = Tr->MaxSpeed[0];
	Tr_3.Speed[1] = Tr->Speed[1];
	Tr_3.Acel[0] = 0;
	Tr_3.Acel[1] = 0;
	Tr_3.Pulses[0] = Tr_2.Pulses[1];
	Calcular_Coef(&Tr_3);
	Tr_3.Pulses[1] = Tr_Pos(Tr_3.Time[1]);

}
float Calcular_Coef(struct PointsData *Tr) {
	struct PointsData Tr_Aux = *Tr;
	Tr_Aux.Time[1] = Tr_Aux.Time[1] - Tr_Aux.Time[0];
	Tr_Aux.Time[0] = 0;
	Tr->a = (2 * Tr_Aux.Time[0] * Tr_Aux.Speed[1] - 2 * Tr_Aux.Time[0] * Tr_Aux.Speed[0] + 2 * Tr_Aux.Time[1] * Tr_Aux.Speed[0] -
		2 * Tr_Aux.Time[1] * Tr_Aux.Speed[1] + Tr_Aux.Acel[0] * pow_int(Tr_Aux.Time[0], 2) + Tr_Aux.Acel[0] * pow_int(Tr_Aux.Time[1], 2) +
		Tr_Aux.Acel[1] * pow_int(Tr_Aux.Time[0], 2) + Tr_Aux.Acel[1] * pow_int(Tr_Aux.Time[1], 2) - 2 * Tr_Aux.Acel[0] * Tr_Aux.Time[0] * Tr_Aux.Time[1] -
		2 * Tr_Aux.Acel[1] * Tr_Aux.Time[0] * Tr_Aux.Time[1]) / (pow_int(Tr_Aux.Time[0], 4) - 4 * pow_int(Tr_Aux.Time[0], 3) * Tr_Aux.Time[1] +
			6 * pow_int(Tr_Aux.Time[0], 2) * pow_int(Tr_Aux.Time[1], 2) - 4 * Tr_Aux.Time[0] * pow_int(Tr_Aux.Time[1], 3) + pow_int(Tr_Aux.Time[1], 4));

	Tr->b = -(Tr_Aux.Acel[0] * pow_int(Tr_Aux.Time[0], 3) + 2 * Tr_Aux.Acel[0] * pow_int(Tr_Aux.Time[1], 3) +
		2 * Tr_Aux.Acel[1] * pow_int(Tr_Aux.Time[0], 3) + Tr_Aux.Acel[1] * pow_int(Tr_Aux.Time[1], 3) -
		3 * pow_int(Tr_Aux.Time[0], 2) * Tr_Aux.Speed[0] + 3 * pow_int(Tr_Aux.Time[0], 2) * Tr_Aux.Speed[1] +
		3 * pow_int(Tr_Aux.Time[1], 2) * Tr_Aux.Speed[0] - 3 * pow_int(Tr_Aux.Time[1], 2) * Tr_Aux.Speed[1] -
		3 * Tr_Aux.Acel[0] * Tr_Aux.Time[0] * pow_int(Tr_Aux.Time[1], 2) - 3 * Tr_Aux.Acel[1] * pow_int(Tr_Aux.Time[0], 2) * Tr_Aux.Time[1]) /
		(pow_int(Tr_Aux.Time[0], 4) - 4 * pow_int(Tr_Aux.Time[0], 3) * Tr_Aux.Time[1] +
			6 * pow_int(Tr_Aux.Time[0], 2) * pow_int(Tr_Aux.Time[1], 2) - 4 * Tr_Aux.Time[0] * pow_int(Tr_Aux.Time[1], 3) + pow_int(Tr_Aux.Time[1], 4));

	Tr->c = (Tr_Aux.Acel[0] * pow_int(Tr_Aux.Time[1], 4) + Tr_Aux.Acel[1] * pow_int(Tr_Aux.Time[0], 4) +
		2 * Tr_Aux.Acel[0] * pow_int(Tr_Aux.Time[0], 3) * Tr_Aux.Time[1] + 2 * Tr_Aux.Acel[1] * Tr_Aux.Time[0] *
		pow_int(Tr_Aux.Time[1], 3) + 6 * Tr_Aux.Time[0] * pow_int(Tr_Aux.Time[1], 2) * Tr_Aux.Speed[0] -
		6 * pow_int(Tr_Aux.Time[0], 2) * Tr_Aux.Time[1] * Tr_Aux.Speed[0] - 6 * Tr_Aux.Time[0] *
		pow_int(Tr_Aux.Time[1], 2) * Tr_Aux.Speed[1] + 6 * pow_int(Tr_Aux.Time[0], 2) * Tr_Aux.Time[1] *
		Tr_Aux.Speed[1] - 3 * Tr_Aux.Acel[0] * pow_int(Tr_Aux.Time[0], 2) * pow_int(Tr_Aux.Time[1], 2) -
		3 * Tr_Aux.Acel[1] * pow_int(Tr_Aux.Time[0], 2) * pow_int(Tr_Aux.Time[1], 2)) / (pow_int(Tr_Aux.Time[0], 4) -
			4 * pow_int(Tr_Aux.Time[0], 3) * Tr_Aux.Time[1] + 6 * pow_int(Tr_Aux.Time[0], 2) * pow_int(Tr_Aux.Time[1], 2) -
			4 * Tr_Aux.Time[0] * pow_int(Tr_Aux.Time[1], 3) + pow_int(Tr_Aux.Time[1], 4));

	Tr->d = (pow_int(Tr_Aux.Time[0], 4) * Tr_Aux.Speed[1] + pow_int(Tr_Aux.Time[1], 4) * Tr_Aux.Speed[0] -
		Tr_Aux.Acel[0] * Tr_Aux.Time[0] * pow_int(Tr_Aux.Time[1], 4) - Tr_Aux.Acel[1] * pow_int(Tr_Aux.Time[0], 4) *
		Tr_Aux.Time[1] - 4 * Tr_Aux.Time[0] * pow_int(Tr_Aux.Time[1], 3) * Tr_Aux.Speed[0] -
		4 * pow_int(Tr_Aux.Time[0], 3) * Tr_Aux.Time[1] * Tr_Aux.Speed[1] + 2 * Tr_Aux.Acel[0] *
		pow_int(Tr_Aux.Time[0], 2) * pow_int(Tr_Aux.Time[1], 3) - Tr_Aux.Acel[0] * pow_int(Tr_Aux.Time[0], 3) *
		pow_int(Tr_Aux.Time[1], 2) - Tr_Aux.Acel[1] * pow_int(Tr_Aux.Time[0], 2) * pow_int(Tr_Aux.Time[1], 3) +
		2 * Tr_Aux.Acel[1] * pow_int(Tr_Aux.Time[0], 3) * pow_int(Tr_Aux.Time[1], 2) + 3 * pow_int(Tr_Aux.Time[0], 2) *
		pow_int(Tr_Aux.Time[1], 2) * Tr_Aux.Speed[0] + 3 * pow_int(Tr_Aux.Time[0], 2) * pow_int(Tr_Aux.Time[1], 2) *
		Tr_Aux.Speed[1]) / (pow_int(Tr_Aux.Time[0], 4) - 4 * pow_int(Tr_Aux.Time[0], 3) * Tr_Aux.Time[1] +
			6 * pow_int(Tr_Aux.Time[0], 2) * pow_int(Tr_Aux.Time[1], 2) - 4 * Tr_Aux.Time[0] *
			pow_int(Tr_Aux.Time[1], 3) + pow_int(Tr_Aux.Time[1], 4));

	Tr->e = 0;

	/*Tr_Aux.Pulses[0] - (1 / 4.0)*Tr_Aux.a*pow_int(Tr_Aux.Time[0], 4) - (1 / 3.0)*Tr_Aux.b*pow_int(Tr_Aux.Time[0], 3) -
	(1 / 2.0)*Tr_Aux.c*pow_int(Tr_Aux.Time[0], 2) - Tr_Aux.d * Tr_Aux.Time[0]; //=0;*/

}

float pow_int(float base, int exp) {
	float aux = base;
	for (int i = 1; i < exp; i++) {
		base = base * aux;
	}
	return base;
}
float Tr_Pos(float time) {
	//long long time = millis_get() - Tr_P.Time_Ini;
	PointsData Tr;

	if ((time >= Tr_1.Time[0]) && (time <= Tr_1.Time[1])) {
		Tr = Tr_1;
	}
	else if ((time > Tr_2.Time[0]) && (time <= Tr_2.Time[1])) {
		Tr = Tr_2;
	}
	else if ((time > Tr_3.Time[0]) && (time <= Tr_3.Time[1])) {
		Tr = Tr_3;
	}
	else
	{
		return Tr_3.Pulses[1];
	}
	time = time - Tr.Time[0];

	//A ver, persona del futuro, cuidado que pulsos es la integral indefinida de vel
	//en el Intervalo 0:x , por lo que se añade el valor inicial Tr.Pulses[0]
	float pulsos = (1 / 4.0)*Tr.a*pow_int(time, 4) + (1 / 3.0)*Tr.b*pow_int(time, 3) +
		(1 / 2.0)*Tr.c*pow_int(time, 2) + Tr.d * time + Tr.Pulses[0];
	return pulsos;
}

float Tr_Vel(float time) {
	//long long time = millis_get() - Tr_P.Time_Ini;
	PointsData Tr;

	if ((time >= Tr_1.Time[0]) && (time <= Tr_1.Time[1])) {
		Tr = Tr_1;
	}
	else if ((time > Tr_2.Time[0]) && (time <= Tr_2.Time[1])) {
		Tr = Tr_2;
	}
	else if ((time > Tr_3.Time[0]) && (time <= Tr_3.Time[1])) {
		Tr = Tr_3;
	}
	else
	{
		return 0;
	}

	time = time - Tr.Time[0];
	float Vel = Tr.a * pow_int(time, 3) + Tr.b * pow_int(time, 2) + Tr.c * time + Tr.d;
	return Vel;
}

float Tr_Acel(float time) {
	//long long time = millis_get() - Tr_P.Time_Ini;
	PointsData Tr;

	if ((time >= Tr_1.Time[0]) && (time <= Tr_1.Time[1])) {
		Tr = Tr_1;
	}
	else if ((time > Tr_2.Time[0]) && (time <= Tr_2.Time[1])) {
		Tr = Tr_2;
	}
	else if ((time > Tr_3.Time[0]) && (time <= Tr_3.Time[1])) {
		Tr = Tr_3;
	}
	else
	{
		return 0;
	}

	time = time - Tr.Time[0];
	float Acel = 3 * Tr.a * pow_int(time, 2) + 2 * Tr.b *time + Tr.c;
	return Acel;
}


float Calcular_Pulsos(struct PointsData *Tr) {
	float pulsos = (1 / 4.0)*Tr->a*(pow_int(Tr->Time[1], 4) - pow_int(Tr->Time[0], 4)) + (1 / 3.0)*Tr->b*(pow_int(Tr->Time[1], 3) - pow_int(Tr->Time[0], 3))
		+ (1 / 2.0)*Tr->c*(pow_int(Tr->Time[1], 2) - pow_int(Tr->Time[0], 2)) + Tr->d * (Tr->Time[1] - Tr->Time[0]);
	pulsos = round(pulsos);

	return pulsos;
}

void print_data() {
	if (data_logging) {
		Serial.print(millis_get());
		Serial.print(",");
		Serial.print(velo);
		Serial.print(",");
		Serial.println(Motor.Vel_F);
	}
}

float seconds_get() {
	return millis_get()*0.001;
}

