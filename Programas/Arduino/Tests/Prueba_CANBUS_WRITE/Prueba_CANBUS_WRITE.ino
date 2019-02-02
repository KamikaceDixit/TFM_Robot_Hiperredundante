#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg1;
MCP2515 mcp2515(10);


void setup() {

	canMsg1.can_id = 0;
	canMsg1.can_dlc = 1;
	canMsg1.data[0] = 0;



	while (!Serial);
	Serial.begin(115200);
	SPI.begin();
	//pinMode(8, OUTPUT);
 pinMode(8,OUTPUT);
	mcp2515.reset();
	mcp2515.setBitrate(CAN_125KBPS);
	mcp2515.setNormalMode();

	Serial.println("Example: Write to CAN");

	//digitalWrite(8, HIGH);
	canMsg1.data[0] = 0;
	//delay(1000);

}

void loop() {
	canMsg1.data[0] = 1;
	mcp2515.sendMessage(&canMsg1);
	digitalWrite(8,HIGH);
	delay(500);
	canMsg1.data[0] = 0;
	mcp2515.sendMessage(&canMsg1);
    digitalWrite(8,LOW);
	delay(500);

	
}
