// PRUEBAS PARA RECIBIR MENSAJES EN BUS CAN

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

    //SISTEMA DE EMERGENCIA
    digitalWrite(LED1, HIGH);
    digitalWrite(EN_CARGA, LOW);
    digitalWrite(EN_BOOST, HIGH);
    digitalWrite(EN_BUCK, LOW); 

}


void loop(){
    
    if (mcp2515.readMessage(&Msg_Prueba) == MCP2515::ERROR_OK) {
        if(Msg_Prueba.can_id == 0){
            for (int i = 0; i < Msg_Prueba.can_dlc; i++){
                Serial.print(Msg_Prueba.data[i]);
                if(Msg_Prueba.data[i] == 1){
                    digitalWrite(LED1, HIGH);
                    Serial.print(Msg_Prueba.data[i]);
                }
                if(Msg_Prueba.data[i] != 1){
                    digitalWrite(LED1, LOW);
                    Serial.print(Msg_Prueba.data[i]);
                }
            }
        }

    Serial.println(" ");

    }
}