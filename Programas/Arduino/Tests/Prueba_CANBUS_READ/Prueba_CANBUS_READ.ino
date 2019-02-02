#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg;

MCP2515 mcp2515(10);
int contador;


void setup() {
	Serial.begin(115200);
	SPI.begin();
		mcp2515.reset();
	mcp2515.setBitrate(CAN_125KBPS);
	mcp2515.setNormalMode();
	pinMode(8, OUTPUT);
	//setPwmFrequency(5, 8);

	//Serial.println("------- CAN Read ----------");
	//Serial.println("ID  DLC   DATA");
}

void loop() {

	if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {

		/*Serial.print(canMsg.can_id, DEC); // print ID
		Serial.print(" ");
		Serial.print(canMsg.can_dlc, DEC); // print DLC
		Serial.print(" ");*/
		if (canMsg.can_id == 0) {
			for (int i = 0; i < canMsg.can_dlc; i++) {  // print the data

				//Serial.print(canMsg.data[i], DEC);
				//Serial.print(" ");
				if ((int)canMsg.data[i] == 1)
				{
					digitalWrite(8, HIGH);
				}
				if ((int)canMsg.data[i] != 1)
				{
					digitalWrite(8, LOW);
				}
			}

		}
		//Serial.println();
	}
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
