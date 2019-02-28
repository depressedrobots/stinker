// this is a conglomeration of several example scripts for the individual sensors of a CMCJU8128.
//
// CCS811 code inspired Adafruit library examples
//
// HDC1080 code inspired by HDC1080_Arduino_Example.ino by Texas Instruments (http://www.ti.com/)

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_CCS811.h>

namespace
{
  // gather data from all sensors every X millis
  const auto READ_INTERVAL = 1000;
  
  // base addresses
  const auto ADDRESS_HDC1080 = 0x40;
  const auto ADDRESS_CCS811  = 0x5A;
  const auto ADDRESS_BMP280  = 0x76;
}

//CCS811 myCCS811(ADDRESS_CCS811);
Adafruit_BMP280 bme;
Adafruit_CCS811 ccs;

void setup()
{
	Serial.begin(9600);
	Wire.begin();

	setupHDC1080();
  setupCCS811();
  setupBMP280();
}

void loop()
{
	double hdc1080Temp     = -1.;
	double hdc1080Humidity = -1.;
	readHDC1080(hdc1080Temp, hdc1080Humidity);

  ccs.setEnvironmentalData(static_cast<uint8_t>(hdc1080Temp), hdc1080Humidity);
  double ccs811Temp    = -1.f;
  uint16_t ccs811CO2  = 0;
  uint16_t ccs811TVOC = 0;
  readCCS811(ccs811Temp, ccs811CO2, ccs811TVOC);

  float bmp280Temp     = -1.f;
  float bmp280Pressure = -1.f;
  readBMP280(bmp280Temp, bmp280Pressure);

  Serial.print("hdc1080Temp: ");
  Serial.println(hdc1080Temp);
  Serial.print("bmp280Temp:  ");
  Serial.println(bmp280Temp);
  Serial.print("ccs811Temp:  ");
  Serial.println(ccs811Temp);
  Serial.print("hdc1080Humidity: ");
  Serial.println(hdc1080Humidity);
  Serial.print("bmp280Pressure:  ");
  Serial.println(bmp280Pressure);
  Serial.print("ccs811TVOC: ");
  Serial.println(ccs811TVOC);
  Serial.print("ccs811CO2:  ");
  Serial.println(ccs811CO2);
  Serial.println();

	delay(READ_INTERVAL);
}

/************************************************************/
/* HDC1080 functions                                        */
/************************************************************/

void setupHDC1080()
{
  Wire.beginTransmission(ADDRESS_HDC1080);
  Wire.write(0x02);
  Wire.write(0x90);
  Wire.write(0x00);
  Wire.endTransmission();

  delay(20);
}

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

/************************************************************/
/* CCS811 functions                                         */
/************************************************************/

void setupCCS811()
{
  if(!ccs.begin())
    Serial.println("Failed to start CCS811 sensor! Please check your wiring.");

  ccs.setDriveMode(CCS811_DRIVE_MODE_1SEC);

  //calibrate temperature sensor
  while(!ccs.available()) {}
  ccs.calculateTemperature();
  ccs.calculateTemperature();
    
  double temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);
}

void readCCS811(double& temperature, uint16_t& co2, uint16_t& tvoc)
{
  if(!ccs.available())
    return;
    
  temperature = ccs.calculateTemperature();
  
  if(!ccs.readData())
  {
    co2  = ccs.geteCO2();
    tvoc = ccs.getTVOC();
  }
  else
    Serial.println("CCS811 ERROR!");
}

/************************************************************/
/* BMP280 functions                                         */
/************************************************************/

void setupBMP280()
{
  if (!bme.begin(ADDRESS_BMP280))
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
}

void readBMP280(float& temperature, float& pressure)
{
  temperature = bme.readTemperature();
  pressure = bme.readPressure();
}
