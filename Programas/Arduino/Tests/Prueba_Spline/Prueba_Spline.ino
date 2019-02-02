#include <spline.h>
#include <SingleEMAFilterLib.h>

Spline tempCurve;
long datos[240] = { 0,0,100,200,370,570,770,1030,1300,1630,1930,2270,2530,2770,2970,3130,3330,3430,3570,3600,3670,3730,3770,3870,3900,3930,3970,4030,4030,4100,4170,4200,4230,4230,4270,4300,4270,4270,4300,4300,4330,4330,4330,4330,4370,4400,4400,4400,4430,4470,4470,4470,4500,4500,4470,4430,4500,4530,4530,4530,4530,4570,4570,4600,4630,4670,4670,4670,4670,4670,4670,4670,4670,4670,4670,4630,4630,4630,4630,4630,4630,4630,4630,4630,4630,4670,4670,4700,4700,4700,4700,4700,4730,4700,4700,4700,4700,4700,4700,4730,4730,4730,4700,4730,4770,4770,4770,4730,4730,4730,4730,4730,4730,4730,4730,4730,4730,4770,4770,4730,4730,4730,4770,4730,4700,4700,4730,4730,4730,4730,4770,4770,4730,4770,4770,4770,4730,4700,4730,4730,4730,4730,4730,4730,4730,4730,4770,4770,4730,4730,4730,4770,4770,4770,4800,4830,4800,4800,4800,4830,4800,4770,4800,4800,4770,4770,4770,4770,4770,4770,4800,4800,4770,4800,4800,4770,4800,4800,4800,4770,4770,4800,4800,4770,4770,4770,4730,4770,4800,4800,4800,4770,4800,4800,4800,4830,4830,4800,4800,4800,4770,4800,4770,4770,4770,4730,4770,4770,4730,4770,4770,4730,4770,4770,4770,4770,4770,4770,4770,4730,4770,4770,4730,4770,4770,4770,4770,4770,4770,4800,4800,4800,4800,4770,4770,4770,4730,4770,4770,4730 };
SingleEMAFilter<int> singleEMAFilter(0.4);


void setup(void) {
	Serial.begin(115200);
	/*
	float x[2] = {0,3};
	float y[2] = {0,8000};
	float m[2] = {0,0};
	float aux = 0;
	tempCurve.setPoints(x, y, m, 2);
	tempCurve.setDegree(Hermite);
	
	for (float i = 0; i <= 3; i += 0.1) {
		float temp = tempCurve.value(i);
		Serial.print(i);
		Serial.print("\t");
		for (float j = 0; j <= temp; j += 100) {
			Serial.print("-");
		}
		Serial.print("   ");
		Serial.println(temp);
	}


	for (float i = 0; i <= 3; i += 0.1) {
		float temp = tempCurve.value_v(i);
		Serial.print(i);
		Serial.print("\t");
		for (float j = 0; j <= abs(temp); j += 100) {
			Serial.print("-");
		}

		Serial.print("   ");
		Serial.println(temp);
	}

	aux = 0;
	for (float i = 0; i <= 3; i += 0.1) {
		float temp = tempCurve.value_a(i);
		Serial.print(i);
		Serial.print("\t");
		for (float j = 0; j <= abs(temp); j += 600) {
			Serial.print("-");
		}
		Serial.print("   ");
		Serial.println(temp);
	}*/

	for (int iCount = 0; iCount < (sizeof(datos)/sizeof(long)); iCount++)
	{
		
		singleEMAFilter.AddValue(datos[iCount]);
		int filteredLP = singleEMAFilter.GetLowPass();
		int filteredHP = singleEMAFilter.GetHighPass();
		Serial.print(datos[iCount]);
		Serial.print(",");
		Serial.println(filteredLP);
		//Serial.print(",");
		//Serial.println(filteredHP);
		delay(5);
	}
}

void loop(void) {


}
