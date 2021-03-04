#include <SPI.h>
#include <mcp2515.h>
#include <millis.h>

                                        // TAREAS
                                        // Ajustar el potenciómetro para salida del Boost y para el NTC, ponerle pegamento después
#define SOS         A0
#define EMG_24      A1                  // Testear placa
#define EMG_9       A2
#define BAT         A3
#define EN_CARGA    2                   // Atornillar placa y colocar en el robot
#define EN_BOOST    3                   // Cablear CAN con las demás placas
#define EN_BUCK     4                   // Subir programa a las demás placas
#define LED1        8                   // Hacer pruebas
#define INT         9                   // PARTY
#define C_CS        10

#define EMG_LENGHT 1        // ENTIENDO QUE VALE CON 1 BYTE
#define EMG_ID 3000         // DEFINIR ID, DEBE SER PRIORITARIO

#define POWER_OFF_LENGHT 1
#define POWER_OFF_ID    //DEFINIR

MCP2515 mcp2515(C_CS);

struct can_frame CanEmgMsg;
float BUS_24V = 0;
float BUS_9V = 0;

float read_24V(){

    int i = 0;
    int lectura[100] = {};
    int media = 0;
    int time = 0;

    while(time<20){                             // Cuento durante 20ms el bus de 24V y devuelvo su media
        time = millis();
        lectura[i] = lectura[i-1] + analogRead(EMG_24);
        i++;
    }

    media = lectura[i] / 100;

    return media;

}

float read_9V(){

    int i = 0;
    int lectura[100] = {};
    int media = 0;
    int time = 0;

    while(time<20){                             // Cuento durante 20ms el bus de 9V y devuelvo su media
        time = millis();
        lectura[i] = lectura[i-1] + analogRead(EMG_9);
        i++;
    }

    media = lectura[i] / 100;

    return media;

}

void setup() {

    // Definición de entradas y salidas digitales

    pinMode(EN_CARGA, OUTPUT);
    pinMode(EN_BOOST, OUTPUT);
    pinMode(EN_BUCK, OUTPUT);
    pinMode(LED1, OUTPUT);
    pinMode(C_CS, OUTPUT);

    // BUS-CAN

    SPI.begin();
    mcp2515.reset();
    mcp2515.setBitrate(CAN_125KBPS);
    mcp2515.setNormalMode();

    CanEmgMsg.can_id = EMG_ID;
    CanEmgMsg.can_dlc = EMG_LENGHT;

    // Inicio de puerto Serie a 115200 baudios

    while(!Serial);
    Serial.begin(115200);

}


// A VER HIJO MÍO, TEN MUCHO CUIDADO, COGE UNA TARDE Y TE ASEGURAS DE TODAS LAS TENSIONES
// QUE SALEN DE TODOS LOS PINES DE LA ELECTRÓNICA DE POTENCIA
// POR FAVOR Y GRACIAS

void loop() {

    BUS_24V = read_24V();                         // Igual no hace falta, se mete la función en el If?
    BUS_9V = read_9V();

    if ((BUS_24V < 24) || (BUS_9V < 9)){        // AJUSTAR EL VALOR

        digitalWrite(LED1, HIGH);               // Activo LED de emergencia
        digitalWrite(EN_CARGA, LOW);            // Desactivo el circuito de carga
        digitalWrite(EN_BOOST, HIGH);           // ESTE ES EL HIJOPUTA, SI SE ENCIENDE A LA VEZ LO MISMO PETA TODO
        digitalWrite(EN_BUCK, LOW);             // El circuito Buck en teoría puede estar encendido siempre


        // Protocolo de comunicación BUS-CAN de emergencia

        // He detectado EMG, envío por CAN mensaje de EMG
        CanEmgMsg.data[0] = 1;                  // Pongo a 1 el primer bit de la trama de datos (8 bits)
        mcp2515.sendMessage(&CanEmgMsg);        // Envío el mensaje

        }
    else{               //IGUAL LO PONGO EN EL SETUP

        digitalWrite(LED1, LOW);
        digitalWrite(EN_CARGA, HIGH);
        digitalWrite(EN_BOOST, LOW);
        digitalWrite(EN_BUCK, HIGH);            // OJO CUIDAO EH
        
    }
    // Leo el bus a la espera de recibir el mensaje de apagado de la Placa Central

    if(mcp2515.readMessage(&CanEmgMsg) == MCP2515::ERROR_OK) {
       // if(CanEmgMsg.can_id == POWER_OFF_ID){
         //   digitalWrite(EN_BOOST, LOW);        // Apago el circuito de alimentación
       // }
    }
}

// Función para leer la tensión del bus durante un corto periodo de tiempo (~ms)
// La tensión no es constante, si solo pongo un IF puede tener falsas medidas
