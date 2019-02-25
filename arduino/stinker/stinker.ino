#include <Wire.h>

#include "SparkFunCCS811.h" // http://librarymanager/All#SparkFun_CCS811

// gather data from all sensors every X millis
const auto READ_INTERVAL = 10000;

// base addresses
const auto ADDRESS_HDC1080 = 0x40;

void setup()
{
	Serial.begin(9600);
	Wire.begin();

	setupHDC1080();
}

void setupHDC1080()
{
	Wire.beginTransmission(ADDRESS_HDC1080);
	Wire.write(0x02);
	Wire.write(0x90);
	Wire.write(0x00);
	Wire.endTransmission();

	delay(20);
}

void loop()
{
	double hdc1080Temp = -1.;
	double hdc1080Humidity = -1.;

	readHDC1080(hdc1080Temp, hdc1080Humidity);

	Serial.print("temp: ");
	Serial.println(hdc1080Temp);
	Serial.print("humidity: ");
	Serial.println(hdc1080Humidity);

	delay(READ_INTERVAL);
}

// inspired by HDC1080_Arduino_Example.ino by Texas Instruments (http://www.ti.com/)
void readHDC1080(double& temperature, double& humidity) 
{
	// request data from temperature + humidity regiser
	Wire.beginTransmission(ADDRESS_HDC1080);
	Wire.write(0x00);
	Wire.endTransmission();
	delay(20);
	Wire.requestFrom(ADDRESS_HDC1080, 4);
	delay(1);

	if (Wire.available() >= 4)
	{
		// temp reading
		uint8_t buffer[4];
		buffer[0] = Wire.read();	// upper byte
		buffer[1] = Wire.read();	// lower byte
		const uint16_t rawTemp = static_cast<uint16_t>(buffer[0]) << 8 | static_cast<uint16_t>(buffer[1]);

		// humidity reading
		buffer[2] = Wire.read();	// upper byte
		buffer[3] = Wire.read();	// lower byte
		const uint16_t rawHumid = static_cast<uint16_t>(buffer[2]) << 8 | static_cast<uint16_t>(buffer[3]);

		// write results into output vars
		temperature = ((static_cast<double>(rawTemp) / 65536.0) * 165) - 40;
		humidity = (static_cast<double>(rawHumid) / 65536.0) * 100.0; 
	}
}
