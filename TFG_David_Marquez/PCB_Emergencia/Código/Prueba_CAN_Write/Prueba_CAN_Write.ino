// PRUEBAS PARA ENVIAR MENSAJES EN BUS CAN

// Bit dominante -> 0
// Bit recesivo -> 1

// Una trama CAN con identificador más bajo (mayor número de bits dominantes en las primeras posiciones) 
// tiene más prioridad que una trama con identificador más alto. 


#include <SPI.h>
#include <mcp2515.h>

#define SOS         A0
#define EMG_24      A1
#define EMG_9       A2
#define BAT         A3
#define EN_CARGA    2
#define EN_BOOST    3
#define EN_BUCK     4
#define LED1        8
#define INT         9
#define C_CS        10

struct can_frame Msg_Prueba;
MCP2515 mcp2515(C_CS);

void setup(){

    //DECLARACIÓN DE PINES DIGITALES
    pinMode(EN_CARGA, INPUT);
    pinMode(EN_BOOST, INPUT);
    pinMode(EN_BUCK, INPUT);
    pinMode(LED1, OUTPUT);
    pinMode(C_CS, OUTPUT);

    //SERIAL
    while(!Serial);
    Serial.begin(115200);

    //BUS-CAN
    SPI.begin();
    mcp2515.reset();
    mcp2515.setBitrate(CAN_125KBPS);
    mcp2515.setNormalMode();

    Msg_Prueba.can_id = 0;              // ID del mensaje: 0 mayor prioridad
    Msg_Prueba.can_dlc = 1;             // Número de bytes de datos en el mensaje (entre 0 y 8)
    Msg_Prueba.data[0] = 0;             // Datos

    //SISTEMA DE EMERGENCIA
    digitalWrite(LED1, HIGH);
    digitalWrite(EN_CARGA, LOW);
    digitalWrite(EN_BOOST, HIGH);
    digitalWrite(EN_BUCK, LOW); 

}


void loop(){

    Msg_Prueba.data[0] = 1;                 //Pongo a 1 el primer bit de la trama de datos (8 bits)
    mcp2515.sendMessage(&Msg_Prueba);       //Envío el mensaje
    
    if(Msg_Prueba.data[0] == 1){
        digitalWrite(LED1,HIGH);            //Apago el LED
    }

    for(int i = 0; i < Msg_Prueba.can_dlc; i++){
        Serial.print(Msg_Prueba.data[i]);
    }
    
    delay(500);
	Msg_Prueba.data[0] = 0;                 //Pongo a 0 el bit
	mcp2515.sendMessage(&Msg_Prueba);       //Envío el mensaje
    
    if(Msg_Prueba.data[0] == 0){
        digitalWrite(LED1,LOW);             //Apago el LED
    }                
               
	delay(500);

    for(int i = 0; i < Msg_Prueba.can_dlc; i++){
        Serial.print(Msg_Prueba.data[i]);
    }
}