// PRUEBAS PARA MEDIR TENSIONES DE LA PLACA

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


int Lectura_24V = 0;
int Lectura_9V = 0;

void setup(){

    pinMode(EN_CARGA, OUTPUT);
    pinMode(EN_BOOST, OUTPUT);
    pinMode(EN_BUCK, OUTPUT);
    pinMode(LED1, OUTPUT);
    pinMode(C_CS, OUTPUT);

    while(!Serial);
    Serial.begin(115200);

    digitalWrite(LED1, HIGH);
    digitalWrite(EN_CARGA, LOW);
    digitalWrite(EN_BOOST, HIGH);
    digitalWrite(EN_BUCK, LOW); 

}


void loop(){

    Lectura_24V = analogRead(EMG_24);
    Lectura_9V  = analogRead(EMG_9);

    Serial.print("Lectura 24V: ");
    Serial.println(Lectura_24V);


}
