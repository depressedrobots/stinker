// this is a conglomeration of several example scripts for the individual sensors of a CMCJU8128.
//
// CCS811 code inspired by "Example6_TwentyMinuteTest" by Sparkfun. Copyright:
// Nathan Seidle @ SparkFun Electronics
// Marshall Taylor @ SparkFun Electronics
//
// HDC1080 code inspired by HDC1080_Arduino_Example.ino by Texas Instruments (http://www.ti.com/)

#include <Wire.h>
#include "SparkFunCCS811.h" // http://librarymanager/All#SparkFun_CCS811

namespace
{
  // gather data from all sensors every X millis
  const auto READ_INTERVAL = 10000;
  
  // base addresses
  const auto ADDRESS_HDC1080 = 0x40;
  const auto ADDRESS_CCS811  = 0x5A;
}

CCS811 myCCS811(ADDRESS_CCS811);

void setup()
{
	Serial.begin(9600);
	Wire.begin();

	setupHDC1080();
  setupCCS811();
}

void loop()
{
	double hdc1080Temp     = -1.;
	double hdc1080Humidity = -1.;
	readHDC1080(hdc1080Temp, hdc1080Humidity);

  myCCS811.setEnvironmentalData(static_cast<float>(hdc1080Temp), static_cast<float>(hdc1080Humidity));
  float ccs811Temp    = -1.f;
  uint16_t ccs811CO2  = 0;
  uint16_t ccs811TVOC = 0;
  readCCS811(ccs811Temp, ccs811CO2, ccs811TVOC);

  Serial.print("hdc1080Humidity: ");
  Serial.println(hdc1080Humidity);
	Serial.print("hdc1080Temp: ");
	Serial.println(hdc1080Temp);
  Serial.print("ccs811Temp: ");
  Serial.println(ccs811Temp);
  Serial.print("ccs811CO2: ");
  Serial.println(ccs811CO2);
  Serial.print("ccs811TVOC: ");
  Serial.println(ccs811TVOC);
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
  CCS811Core::status returnCode = myCCS811.begin();
  Serial.print("CCS811::begin() exited with: ");
  printDriverError( returnCode );
  Serial.println();
  myCCS811.setDriveMode(2); // every 10s
}

void readCCS811(float& temperature, uint16_t& co2, uint16_t& tvoc)
{
  if(!myCCS811.dataAvailable())
  {
    if (myCCS811.checkForStatusError())
      printSensorError();

    return;
  }

  myCCS811.readAlgorithmResults();
  myCCS811.readNTC();
  temperature = myCCS811.getTemperature();
  co2 = myCCS811.getCO2();
  tvoc = myCCS811.getTVOC();
}

void printDriverError( const CCS811Core::status errorCode )
{
  switch ( errorCode )
  {
    case CCS811Core::SENSOR_SUCCESS:
      Serial.print("SUCCESS");
      break;
    case CCS811Core::SENSOR_ID_ERROR:
      Serial.print("ID_ERROR");
      break;
    case CCS811Core::SENSOR_I2C_ERROR:
      Serial.print("I2C_ERROR");
      break;
    case CCS811Core::SENSOR_INTERNAL_ERROR:
      Serial.print("INTERNAL_ERROR");
      break;
    case CCS811Core::SENSOR_GENERIC_ERROR:
      Serial.print("GENERIC_ERROR");
      break;
    default:
      Serial.print("Unspecified error.");
  }
}

void printSensorError()
{
  const auto error = myCCS811.getErrorRegister();

  if ( error == 0xFF ) //comm error
    Serial.println("Failed to get ERROR_ID register.");
  else
  {
    Serial.print("CCS811 Error: ");
    if (error & 1 << 5) Serial.print("HeaterSupply");
    if (error & 1 << 4) Serial.print("HeaterFault");
    if (error & 1 << 3) Serial.print("MaxResistance");
    if (error & 1 << 2) Serial.print("MeasModeInvalid");
    if (error & 1 << 1) Serial.print("ReadRegInvalid");
    if (error & 1 << 0) Serial.print("MsgInvalid");
    Serial.println();
  }
}
