#include <SPI.h>
#include <mcp2515.h>
#include <SingleEMAFilterLib.h>
                                        // TAREAS
                                        // Ajustar el potenciómetro para salida del Boost y para el NTC, ponerle pegamento después
#define SOS         A0
#define EMG_24      A1                  // Testear placa
#define EMG_9       A2
#define BAT         A3
#define EN_CARGA    2                   
#define EN_BOOST    3                   // Cablear CAN con las demás placas
#define EN_BUCK     4                   // Subir programa a las demás placas
#define LED1        8                   // Hacer pruebas
#define INT         9                   // PARTY
#define C_CS        10

#define EMG_LENGHT 1        // ENTIENDO QUE VALE CON 1 BYTE
#define EMG_ID 3000         // DEFINIR ID, DEBE SER PRIORITARIO

#define POWER_OFF_LENGHT 1
#define POWER_OFF_ID 3001    //DEFINIR

#define R_FACTOR_24V  0.0278161
#define R_FACTOR_9V   0.0099063
#define R_FACTOR_BATTERY   5/1023

MCP2515 mcp2515(C_CS);

SingleEMAFilter<float> singleEMAFilter24V(0.6);
SingleEMAFilter<float> singleEMAFilter9V(0.6);

struct can_frame CanEmgMsg;
float BUS_24V = 0;
float BUS_9V = 0;
float BATTERY_LEVEL = 0;
boolean SOS_FLAG = false;

float read_24V() {
  float lectura = {};
  lectura = analogRead(EMG_24);
  singleEMAFilter24V.AddValue(lectura);                    //Filtro EMA Paso Bajo
  return singleEMAFilter24V.GetLowPass() * R_FACTOR_24V;
}

float read_9V() {                 
  float lectura = {};
  lectura = analogRead(EMG_9);
  singleEMAFilter9V.AddValue(lectura);                      //Filtro EMA Paso Bajo
  return singleEMAFilter9V.GetLowPass() * R_FACTOR_9V;
}

float read_Battery() {
  float lectura = {};
  lectura = analogRead(BAT);
  return lectura * R_FACTOR_BATTERY;
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

  while (!Serial);
  Serial.begin(115200);
  delay(500);
}

void loop() {

  // Lectura de buses de tensión, 9, 24 y batería
  BUS_24V = read_24V();
  BUS_9V = read_9V();
  BATTERY_LEVEL = read_Battery();
  if(BUS_24V < 20){
      //Envío señal de emergencia a la Placa Central, pero no alimento por batería.
    
    }

  if (BUS_9V <= 7) {       // Aqui ha entrado en caso de emergencia, la tension se esta yendo a la mierda
    SOS_FLAG = true;
    Serial.println("EMERGENCIA");
    Serial.println("Medida24V: " + String(BUS_24V) + ", Medida9V: " + String(BUS_9V) + ", MedidaBat: " + String(BATTERY_LEVEL));
    digitalWrite(LED1, HIGH);               // Activo LED de emergencia
    digitalWrite(EN_CARGA, LOW);            // Desactivo el circuito de carga
    digitalWrite(EN_BOOST, HIGH);           // ESTE ES EL HIJOPUTA, SI SE ENCIENDE A LA VEZ LO MISMO PETA TODO
    digitalWrite(EN_BUCK, HIGH);             // Logica Negada, 5V-> apagado
    delay(200);

    // Protocolo de comunicación BUS-CAN de emergencia

    // He detectado EMG, envío por CAN mensaje de EMG
    CanEmgMsg.data[0] = 1;                  // Pongo a 1 el primer bit de la trama de datos (8 bits)
    mcp2515.sendMessage(&CanEmgMsg);        // Envío el mensaje
  }
  else if (BUS_9V >= 9 ) {
    // Normal State
    Serial.println("NORMAL");
    Serial.println("Medida24V: " + String(BUS_24V) + ", Medida9V: " + String(BUS_9V) + ", MedidaBat: " + String(BATTERY_LEVEL));
    digitalWrite(LED1, LOW);
    digitalWrite(EN_CARGA, HIGH);
    digitalWrite(EN_BOOST, LOW);
    digitalWrite(EN_BUCK, LOW);
    if (SOS_FLAG == true) {
      delay(200);
      SOS_FLAG = false;
    }
  }
  else {
    Serial.println("Medida24V: " + String(BUS_24V) + ", Medida9V: " + String(BUS_9V) + ", MedidaBat: " + String(BATTERY_LEVEL));
    //delay(200);
    //Aqui estamos tirando con la bateria
  }
  // Leo el bus a la espera de recibir el mensaje de apagado de la Placa Central

  if (mcp2515.readMessage(&CanEmgMsg) == MCP2515::ERROR_OK) {
    if (CanEmgMsg.can_id == POWER_OFF_ID) {
      //digitalWrite(EN_BOOST, LOW);        // Apago el circuito de alimentación
    }
  }
}

// Función para leer la tensión del bus durante un corto periodo de tiempo (~ms)
// La tensión no es constante, si solo pongo un IF puede tener falsas medidas
