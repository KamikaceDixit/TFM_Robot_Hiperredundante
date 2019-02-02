#include <SPI.h>
#include <mcp2515.h>
#include <SingleEMAFilterLib.h>

//Parametros para la identificacion de placa Driver Unica
#define IDs_per_Driver 50	
#define Motors_Number 36	//Numero de motores maximos en el actuador
#define Inst_Mot 24			//Numero de motores instalados en el actuador
#define Config_Lines 10		//Numero de lineas reservadas para la configuracion
#define CAN_Tr_Data_Space 8 // Direcciones desde la posicion 1 de la trayectoria hasta la siguiente ej: 0 - 7

#define Max_Puntos_Por_Trayectoria 2

//Definicion Pines de entrada

#define SOS 32
#define INT 33
#define MISO 50
#define MOSI 51
#define SCK 52
#define C_CS 53

#define LED1 11
#define LED2 12
#define LED3 13

#define BT_STATE 4
#define BT_RX 5
#define BT_TX 6
#define BT_EN 7

#define SDA 20
#define SCL 21

#define Aux_1 A3
#define Aux_2 A2
#define Aux_3 A1
#define Aux_4 A0
#define Aux_5 3
#define Aux_6 2
#define Aux_7 9
#define Aux_8 8
#define Aux_9 23
#define Aux_10 24
#define Aux_11 25
#define Aux_12 26
#define Aux_13 27
#define Aux_14 28
#define Aux_15 29
#define Aux_16 30



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

#define CAN_Pos_Mot_ID 49
#define CAN_Pos_Mot_Len 4



typedef union
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


MCP2515 mcp2515(C_CS);
byte state_FdC[5] = { 0 };
boolean own_FdC = false; //Estado del FdC asociado a la placa



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



//Variables Trayectoria
PointsData Tr_P;	//Definicion de la trayectoria a partir de los puntos recibidos por Bus CAN
					//Para las trayectorias, en pulsos y milisegundos pls


boolean Tr_Recibida[Motors_Number] = { false };
int Suma_Tr_Recibida = 0;
boolean Pos_Alcanzada[Motors_Number] = { false };
float Pos_Motor[Motors_Number] = { 0 }; //Posicion del motor en pulsos de encoder.
boolean Pos_Recibidas_m[Motors_Number] = { false }; //Numero del motor del cual se han recibido las posiciones.
int Pos_Recibidas = 0; //Numero de Posiciones que se han recibido, en teoria hasta 24.
float Pos_Motor_Destino[Motors_Number] = { 0 };
float Vel_Motor_Max[Motors_Number] = { 0 };
float Pos_Diferencia[Motors_Number] = { 0 };
float Configuracion[Config_Lines] = { 0 }; //Vmax, AcelTime, DcelTime, Time...
float Reductora_Motor[Motors_Number] = { 51,27,0,27,27, 0,51,27,0,27, 27,0,51,27,0, 27,27,0,51,27, 0,27,27,0,51,
										 27,0,27,27,0, 51,27,0,27,27, 0 };		//3,6,9,12,15,18,21,24,27,30,33,36
int Mot_I[Inst_Mot] = { 1,2,4,5,7,8,10,11,13,14,16,17,19,20,22,23,25,26,28,29,31,32,34,35 };

void setup() {
	//Definicion de los pines de entrada
	//Los encoder se definen dentro de la libreria
	pinMode(SOS, INPUT);
	pinMode(LED1, OUTPUT);
	pinMode(LED2, OUTPUT);
	pinMode(LED3, OUTPUT);
	pinMode(INT, OUTPUT);
	pinMode(C_CS, OUTPUT);


	Serial.begin(115200);
	while (Serial.available()) { Serial.read(); }
	Serial.setTimeout(200);

	//Iniciar Bus-CAN
	SPI.begin();	//Inicio SPI para comunicacion con el chip Bus-CAN
	mcp2515.reset();
	mcp2515.setBitrate(CAN_125KBPS);
	mcp2515.setNormalMode();

	//Configuracion por defecto
	Configuracion[0] = 1000;//Vmax
	Configuracion[1] = 0.5;//AcelTime
	Configuracion[2] = 0.5;//DecelTime
	Configuracion[3] = 0;//Time
}


int incomingByte = 0;   // for incoming serial data
boolean data_logging = false;
int dt = 0; //Diferencia de tiempos con el ultimo muestreo.
int Trayectoria_Iniciada = 0; //Una vez que comienza la trayectoria se pone a 1.
long tant = 0;
char S_Byte;
char buffer[16];
int size = -1;

void loop() {
	if (millis() - tant >= 1000) {
		tant = millis();
		//Temporizador
	}


	//Escucha Serial Matlab
	if (Serial.available() > 0) {

		memset(buffer, 0, sizeof(buffer)); //Se vacia el buffer
		size = Serial.readBytesUntil('\n', buffer, 16); //Se lee la linea de encabezado, a ver que es

		//Rececpcion de las trayectorias de los motores (Vmax)
		if (strcmp(buffer, "#Tr_1") == 0) {
			Rec_Vel_Max();
			for (int i = 0; i < Inst_Mot; ++i)
			{
				Gen_Tr_Msg(&Tr_P, Mot_I[i]);
				Enviar_Tr(&Tr_P, Mot_I[i]);
				Read_BusCAN();
			}
			long long Tant_aux = millis();
			while (Suma_Tr_Recibida < 24) {
				Read_BusCAN();
				if (millis() - Tant_aux >= 2000) {

					Serial.println(Suma_Tr_Recibida); break;
				}
			}
			if (Suma_Tr_Recibida == 24) { Serial.println("OK"); }
			Suma_Tr_Recibida = 0;

		}

		//Recepcion de la señal para comenzar el movimiento.
		if (strcmp(buffer, "#Ini") == 0) {
			Iniciar_Movimientos();
		}

		//Peticion para enviar las posiciones actuales de los motores.
		if (strcmp(buffer, "#Pos") == 0) {
			Update_Posicion_Motor(0); //Si el valor de entrada es 0 actualiza todos, en caso de ser diferente actualiza el valor introducido.
			for (int i = 0; i < Inst_Mot; ++i) {
				Serial.println(Pos_Motor[Mot_I[i] - 1]);
			}
			//memset(Pos_Motor, 0, sizeof(Pos_Motor));
			//Control de errores
			Serial.println("Errors");
			Serial.println(Inst_Mot - Pos_Recibidas);
			//Identificacion de placas problematicas
			if (Inst_Mot - Pos_Recibidas > 0) {
				boolean AuxSerial = false;
				for (int i = 0; i < Inst_Mot; i++) {
					if (Pos_Recibidas_m[Mot_I[i] - 1] != true) {
						if (AuxSerial) { Serial.print(" - "); }
						Serial.print(Mot_I[i]);
						AuxSerial = true;
					}
				}
				Serial.print('\n');
			}
			memset(Pos_Recibidas_m, 0, sizeof(Pos_Recibidas_m));
		}

		//Recepcion de la configuracion global de todas las trayectorias.
		if (strcmp(buffer, "#Conf_1") == 0) {
			Rec_Config();
		}
	}

	//Escucha Bus-CAN
	Read_BusCAN();
}



//---------------------------------------------------------------------
//---------------------------------------------------------------------
void Read_BusCAN() {
	//Antes habia un if en vez de un while;
	while (mcp2515.readMessage(&ReadMsg) == MCP2515::ERROR_OK) {
		if (ReadMsg.can_id < 1800)//El id pertenece a alguna de las placas de los driver
		{
			Recibir_Msg_Driver();
		}
		else if (ReadMsg.can_id >= 2000 && ReadMsg.can_id <= 2010)//El id pertenece a la placa Fdc
		{

		}
		else if (ReadMsg.can_id >= 1800 && ReadMsg.can_id < 1900)//El id pertenece a la placa Main
		{

		}
	}
}

void Rec_Vel_Max() {
	for (int i = 0; i <= Inst_Mot; ++i) { //Posiciones de todos los motores + Final de Mensaje
		memset(buffer, 0, sizeof(buffer));//Vacio el buffer
		size = Serial.readBytesUntil('\n', buffer, 16);
		if (strcmp(buffer, "#Tr_#") == 0) { break; }
		if (size == 0) { break; }
		Vel_Motor_Max[Mot_I[i] - 1] = atof(buffer);
	}

}

void Rec_Config() {
	for (int i = 0; i <= Config_Lines; ++i) { //Lineas de configuracion + Final de Mensaje
		memset(buffer, 0, sizeof(buffer));
		size = Serial.readBytesUntil('\n', buffer, 16);
		if (strcmp(buffer, "#Conf_#") == 0) { break; }
		if (size == 0) { Serial.println("Configuracion Incompleta"); break; }
		if (i == Config_Lines) { Serial.println("Demasiados Argumentos"); break; }//Si llega aqui es que no ha parado, y el mensaje es muy largo.
		Configuracion[i] = atof(buffer);
	}
}

void Update_Posicion_Motor(int motor) {
	long long Tant = millis();
	Pos_Recibidas = 0;
	BoolUnion AuxB = { 0 };
	AuxB.Valor = true;
	memcpy(WriteMsg.data, AuxB.Cadena, WriteMsg.can_dlc);
	WriteMsg.can_dlc = 1;

	if (motor == 0) {
		for (int i = 0; i < Inst_Mot; ++i) {
			WriteMsg.can_id = CAN_Pos_Mot_ID + IDs_per_Driver * (Mot_I[i] - 1);;
			while (mcp2515.sendMessage(&WriteMsg) != 0);
			Read_BusCAN();
		}
		while (Pos_Recibidas < Inst_Mot) {
			Read_BusCAN();
			if (millis() - Tant > 1000) { break; }
		}
	}
	else {
		WriteMsg.can_id = CAN_Pos_Mot_ID + IDs_per_Driver * (motor - 1);;
		while (mcp2515.sendMessage(&WriteMsg) != 0);
		while (Pos_Recibidas == 0) {
			Read_BusCAN();
			if (millis() - Tant > 1000) { break; }
		}
	}
}

void Iniciar_Movimientos() {
	BoolUnion AuxB = { 0 };
	AuxB.Valor = true;
	WriteMsg.can_dlc = 1;
	memcpy(WriteMsg.data, AuxB.Cadena, WriteMsg.can_dlc);
	WriteMsg.can_id = 1800;
	while (mcp2515.sendMessage(&WriteMsg) != 0);
}

void Gen_Tr_Msg(struct PointsData *Tr, int Driver) {
	Tr->AcelTime[0] = Configuracion[1];
	Tr->DcelTime[0] = Configuracion[2];
	Tr->MaxSpeed[0] = Vel_Motor_Max[Driver - 1];
	Tr->Time[0] = 0;
	Tr->Time[1] = Configuracion[3];
	Tr->Speed[0] = 0;
	Tr->Speed[1] = 0;
	Tr->Pulses[0] = Pos_Motor[Driver - 1];
	Tr->Pulses[1] = Pos_Motor_Destino[Driver - 1]; //No sirve para nada, los pulsos se consiguen a partir de la integral.
}

void Enviar_Tr(struct PointsData *Tr, int Driver) {
	FloatUnion AuxF = { 0 };
	BoolUnion AuxB = { 0 };
	//---------------------

	for (int i = 0; i < Tr->Points; i++)
	{
		AuxF.Valor = Tr->Pulses[i];
		WriteMsg.can_dlc = CAN_Tr_Pos_Len;
		memcpy(WriteMsg.data, AuxF.Cadena, WriteMsg.can_dlc);
		WriteMsg.can_id = CAN_Tr_Pos_ID + i * CAN_Tr_Data_Space + IDs_per_Driver * (Driver - 1);
		while (mcp2515.sendMessage(&WriteMsg) != 0);

		if (Tr->Speed[i] != 0) {
			AuxF.Valor = Tr->Speed[i];
			WriteMsg.can_dlc = CAN_Tr_Vel_Len;
			memcpy(WriteMsg.data, AuxF.Cadena, WriteMsg.can_dlc);
			WriteMsg.can_id = CAN_Tr_Vel_ID + i * CAN_Tr_Data_Space + IDs_per_Driver * (Driver - 1);
			while (mcp2515.sendMessage(&WriteMsg) != 0);
		}
		if (Tr->Time[i] != 0) {
			AuxF.Valor = Tr->Time[i];
			WriteMsg.can_dlc = CAN_Tr_Time_Len;
			memcpy(WriteMsg.data, AuxF.Cadena, WriteMsg.can_dlc);
			WriteMsg.can_id = CAN_Tr_Time_ID + i * CAN_Tr_Data_Space + IDs_per_Driver * (Driver - 1);
			while (mcp2515.sendMessage(&WriteMsg) != 0);
		}
	}

	for (int i = 0; i < Tr->Points - 1; i++)
	{
		if (Tr->AcelTime[i] != 0.6) {
			AuxF.Valor = Tr->AcelTime[i];
			WriteMsg.can_dlc = CAN_Tr_Acelt_Len;
			memcpy(WriteMsg.data, AuxF.Cadena, WriteMsg.can_dlc);
			WriteMsg.can_id = CAN_Tr_Acelt_ID + i * CAN_Tr_Data_Space + IDs_per_Driver * (Driver - 1);
			while (mcp2515.sendMessage(&WriteMsg) != 0);
		}
		if (Tr->DcelTime[i] != 0.6) {
			AuxF.Valor = Tr->DcelTime[i];
			WriteMsg.can_dlc = CAN_Tr_Dcelt_Len;
			memcpy(WriteMsg.data, AuxF.Cadena, WriteMsg.can_dlc);
			WriteMsg.can_id = CAN_Tr_Dcelt_ID + i * CAN_Tr_Data_Space + IDs_per_Driver * (Driver - 1);
			while (mcp2515.sendMessage(&WriteMsg) != 0);
		}
		AuxF.Valor = Tr->MaxSpeed[i];
		WriteMsg.can_dlc = CAN_Tr_Vel_Max_Len;
		memcpy(WriteMsg.data, AuxF.Cadena, WriteMsg.can_dlc);
		WriteMsg.can_id = CAN_Tr_Vel_Max_ID + i * CAN_Tr_Data_Space + IDs_per_Driver * (Driver - 1);
		while (mcp2515.sendMessage(&WriteMsg) != 0);
	}
	//Mensaje Final de Tr
	//A ver polletes, que esta es para nota. La función sendMessageTx no está en la libreria original,
	//con ella se puede seleccionar el buffer Tx del MCP2515 por el cual se quiere transmitir. El MCP2515
	//tiene 3 buffers de envio, y al no haber definido prioridades en dichos buffers, se sigue la prioridad
	//por defecto, en la cual el TXB0 tiene la menor prioridad y TXB1 la mayor.
	//Las prioridades son la risa, si hay varios mensajes pendientes por enviar en los buffer, se envia
	//primero el de mayor prioridad. Enviando el mensaje de fin de trayectoria por el buffer de menor
	//prioridad (TXB0), se asegura que todos los demas mensajes hayan sido enviados antes de enviar el de 
	//cierre.
	AuxB.Valor = true;
	WriteMsg.can_dlc = CAN_Tr_Env_Len;
	memcpy(WriteMsg.data, AuxF.Cadena, WriteMsg.can_dlc);
	WriteMsg.can_id = CAN_Tr_Env_ID + IDs_per_Driver * (Driver - 1);
	while (mcp2515.sendMessageTx(mcp2515.TXB0, &WriteMsg) != 0);

}

void Recibir_Msg_Driver() {
	FloatUnion AuxF = { 0 };
	LongUnion AuxL = { 0 };
	BoolUnion AuxB = { 0 };

	//Ya sabemos que los ID's son de trayectoria y corresponden a esta placa, 
	//por lo que lo referenciamos a 0 para facilitar el manejo
	//Se aprovecha la division de enteros, ej: Can_id=941;>>941-50*(941/50)=41
	int ID_Aux = ReadMsg.can_id - IDs_per_Driver * (ReadMsg.can_id / IDs_per_Driver);

	switch (ID_Aux)
	{
	case CAN_Corriente_ID:
		memcpy(AuxF.Cadena, ReadMsg.data, ReadMsg.can_dlc);
		Serial.print("Corriente Driver: ");
		Serial.print(ReadMsg.can_id / IDs_per_Driver);
		Serial.println(AuxF.Valor, 3);
		break;
	case CAN_Dato_Recv_ID:
		memcpy(AuxB.Cadena, ReadMsg.data, ReadMsg.can_dlc);
		Serial.print("Dato Recibido Driver: ");
		Serial.print(ReadMsg.can_id / IDs_per_Driver);
		Serial.println(AuxB.Valor);
		break;
	case CAN_Pos_Alc_ID:
		memcpy(AuxB.Cadena, ReadMsg.data, ReadMsg.can_dlc);
		// Se actualiza el estado de las posiciones alcanzadas.
		Pos_Alcanzada[ReadMsg.can_id / IDs_per_Driver] = true;
		Serial.print("Pos Alc Driver: ");
		Serial.print(ReadMsg.can_id / IDs_per_Driver);
		Serial.println(AuxB.Valor);
		break;
	case CAN_Tr_Recv_ID:
		memcpy(AuxB.Cadena, ReadMsg.data, ReadMsg.can_dlc);
		// Se actualiza el estado de trayectorias recibidas.
		Tr_Recibida[ReadMsg.can_id / IDs_per_Driver] = true;
		Suma_Tr_Recibida += 1;
		break;
	case CAN_Pos_Mot_ID:
		memcpy(AuxF.Cadena, ReadMsg.data, ReadMsg.can_dlc);
		Pos_Motor[ReadMsg.can_id / IDs_per_Driver] = AuxF.Valor;
		Pos_Recibidas_m[ReadMsg.can_id / IDs_per_Driver] = true;
		Pos_Recibidas += 1;
		break;
	default:
		break;
	}
}

void Motor_Current(int motor) {
	WriteMsg.can_dlc = 1;
	WriteMsg.data[0] = 1;
	WriteMsg.can_id = CAN_Corriente_ID + IDs_per_Driver * (motor - 1);
	while (mcp2515.sendMessage(&WriteMsg) != 0);
	//memcpy(WriteMsg.data, Aux.Cadena, WriteMsg.can_dlc);
}
/*
void save_FdC() {
	memcpy(state_FdC, ReadMsg.data, ReadMsg.can_dlc); //Copia ReadMsg.data en State_FdC
	//Compara si el FdC pulsado corresponde a esta placa
	if ((state_FdC[(DriverID - 1) / 8] & (0b00000001 << (DriverID - 1) % 8)) != 0)
	{
		own_FdC = true; //Se recibe el mensaje con los estados de los FdC, si el resultado
		//es positivo signifca que el FdC de esta placa está pulsado.
	}
	else { own_FdC = false; }
}*/

float pow_int(float base, int exp) {
	float aux = base;
	for (int i = 1; i < exp; i++) {
		base = base * aux;
	}
	return base;
}

float Calcular_Pulsos(struct PointsData *Tr) {
	float pulsos = (1 / 4.0)*Tr->a*(pow_int(Tr->Time[1], 4) - pow_int(Tr->Time[0], 4)) + (1 / 3.0)*Tr->b*(pow_int(Tr->Time[1], 3) - pow_int(Tr->Time[0], 3))
		+ (1 / 2.0)*Tr->c*(pow_int(Tr->Time[1], 2) - pow_int(Tr->Time[0], 2)) + Tr->d * (Tr->Time[1] - Tr->Time[0]);
	pulsos = round(pulsos);

	return pulsos;
}