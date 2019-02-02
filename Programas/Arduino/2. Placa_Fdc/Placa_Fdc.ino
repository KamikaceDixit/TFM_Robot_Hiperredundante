#include <SPI.h>
#include <mcp2515.h>

#define Serial_Debugger 0
#define Serial_Debugger2 0
#define S1 2
#define S2 3
#define S3 4
#define S4 5
#define EN 6
#define Salida_Mux_1 A1
#define Salida_Mux_2 A2
#define SOS A0
#define LED1 8
#define INT 9
#define C_CS 10

#define FDC_LENGTH 5
#define FDC_ID 2000

#define UPDATE_REQ_LENGTH 1
#define UPDATE_REQ_ID 2001


typedef union
{
	float number;
	bool bytes[4];
} FloatUnion;


struct can_frame CanFdCMsg;
struct can_frame ReadMsg;

MCP2515 mcp2515(C_CS);
int FdC_Placa[33] = { 0 };
int FdC_Bus[37] = { 0 };
int suma = 0;
byte sended_fdc[5] = { 0 };
byte send_fdc[5] = { 0 };




void setup() {
	//Definicion de los pines de entrada
	pinMode(S1, OUTPUT);
	pinMode(S2, OUTPUT);
	pinMode(S3, OUTPUT);
	pinMode(S4, OUTPUT);
	pinMode(EN, OUTPUT);
	pinMode(LED1, OUTPUT);
	pinMode(C_CS, OUTPUT);
	pinMode(Salida_Mux_1, INPUT);
	pinMode(Salida_Mux_2, INPUT);


	//Definicion mensajes CAN
	CanFdCMsg.can_id = FDC_ID;
	CanFdCMsg.can_dlc = FDC_LENGTH;



	SPI.begin();

	while (!Serial);
	Serial.begin(115200);

	mcp2515.reset();
	mcp2515.setBitrate(CAN_125KBPS);
	mcp2515.setNormalMode();
}

void loop() {
	//Escucha la direccion CAN de peticiones de estado de los FdC
	if (mcp2515.readMessage(&ReadMsg) == MCP2515::ERROR_OK) {
		if (ReadMsg.can_id == UPDATE_REQ_ID) {
			for (int i = 0; i < ReadMsg.can_dlc; i++) {  // print the data
				if (ReadMsg.data[i] == 0b00000001)
				{
					mcp2515.sendMessage(&CanFdCMsg);
					memcpy(sended_fdc, send_fdc, sizeof(send_fdc));
					Serial.println("Posiciones Enviadas");
				}
			}
		}
	}

	//------------------------------------------------------------------------------------
	//For para leer todos los FdC conectados a los 2 MUX
	digitalWrite(EN, LOW);
	for (int s4 = 0; s4 <= 1; s4++) {
		for (int s3 = 0; s3 <= 1; s3++) {
			for (int s2 = 0; s2 <= 1; s2++) {
				for (int s1 = 0; s1 <= 1; s1++) {
					digitalWrite(S4, s4);
					digitalWrite(S3, s3);
					digitalWrite(S2, s2);
					digitalWrite(S1, s1);
					if (digitalRead(Salida_Mux_1)) {
						FdC_Placa[1 + s4 * 8 + s3 * 4 + s2 * 2 + s1] = 1;
					}
					else { FdC_Placa[1 + s4 * 8 + s3 * 4 + s2 * 2 + s1] = 0; }

					if (digitalRead(Salida_Mux_2)) {
						FdC_Placa[17 + s4 * 8 + s3 * 4 + s2 * 2 + s1] = 1;
					}
					else { FdC_Placa[17 + s4 * 8 + s3 * 4 + s2 * 2 + s1] = 0; }
				}
			}
		}
	}
	digitalWrite(EN, HIGH);
	//------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------
	//For para ubicar las entradas de la placa FdC en el vector de señales FdC para el Bus
	suma = 0;
	for (int i = 1; i < 25; i++) {
		FdC_Bus[i + suma] = FdC_Placa[i];
		if (Serial_Debugger) {
			if (FdC_Bus[i + suma] == 1) {
				Serial.print("FdC ");
				Serial.print(i + suma);
				Serial.print(": ");
				Serial.println(FdC_Bus[i + suma]);
			}
		}
		if ((i % 2) == 0) {
			suma = suma + 1;
		}
	}
	//------------------------------------------------------------------------------------


	//------------------------------------------------------------------------------------
	//Transforma el vector FdC_Bus en bits para ser enviado mediante Bus-CAN
	//El bit menos significativo de cada byte es es FdC con numero menor
	for (int i = 0; i <= 4; i++) {
		for (int j = 1; j <= 8; j++) {
			if (FdC_Bus[8 * i + j]) {
				bitWrite(send_fdc[i], j - 1, 1);
			}
			else { bitWrite(send_fdc[i], j - 1, 0); }
			if (8 * i + j > 35) {
				break;
			}
		}
		CanFdCMsg.data[i] = send_fdc[i];
	}

	//------------------------------------------------------------------------------------
	//Envia el mensaje con la posicion de los encoder por Bus-CAN cada vez que cambia.
	if (memcmp(send_fdc, sended_fdc, sizeof(send_fdc))) { //CUIDADO, devuelve 0 sin son iguales.
		mcp2515.sendMessage(&CanFdCMsg);
		memcpy(sended_fdc, send_fdc, sizeof(send_fdc));
		//Serial.println((CanFdCMsg.data[0] & 0b00000010),BIN);
	}

	if (Serial_Debugger2) {
		prntBits(send_fdc[4]);
		prntBits(send_fdc[3]);
		prntBits(send_fdc[2]);
		prntBits(send_fdc[1]);
		prntBits(send_fdc[0]);
		Serial.println();
	}


}


void prntBits(int b)
{
	for (int i = 7; i >= 0; i--)
	{
		Serial.print(bitRead(b, i));
	}
	Serial.print("  ");
}


