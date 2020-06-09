#include <LSM6DSOSensor.h>
#include <LIS2DW12Sensor.h>
#include <LIS2MDLSensor.h>
#include <LPS22HHSensor.h>
#include <STTS751Sensor.h>
#include <HTS221Sensor.h>
#include "teseo_liv3f_class.h"
#include <ArduinoJson.h>
#include "LoRaWAN.h"

const char *devEui = "YOUR_DEVEUI";
const char *appEui = "YOUR_APPEUI";
const char *appKey = "YOUR_APPKEY";

TeseoLIV3F *gps;
int incomingByte;
GNSSParser_Data_t data;
char command[32] = {0};
char msg[256];
int cmdType = 0;
uint32_t t, s;
int tracked;
GPGGA_Info_t stored_positions[64];
int status = 0;
uint32_t stime = 0;
int waitType = 0;

#define MSG_SZ 256
#define waitForRequest 0
#define waitForAnswer 1

#ifdef ARDUINO_SAM_DUE
#define DEV_I2C Wire1
#elif defined(ARDUINO_ARCH_STM32)
#define DEV_I2C Wire
#elif defined(ARDUINO_ARCH_AVR)
#define DEV_I2C Wire
#else
#define DEV_I2C Wire
#endif

#ifdef ARDUINO_SAM_DUE
#include <avr/dtostrf.h>
#elif ARDUINO_STM32L0_B_L072Z_LRWAN1
#include <avr/dtostrf.h>
#endif

// Components
LSM6DSOSensor *AccGyr;
LIS2DW12Sensor *Acc2;
LIS2MDLSensor *Mag;
LPS22HHSensor *PressTemp;
HTS221Sensor *HumTemp;
STTS751Sensor *Temp3;

unsigned long time1;
unsigned long time2;

String MyJson="{";

void setup() {
  StaticJsonDocument<200> doc;
  // Led.
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize serial for output.
  Serial.begin(115200);  
  // Initialize I2C bus.
  DEV_I2C.begin();
  Serial1.begin(9600);
  Serial.println("Setup begin");
  //Create the device object passing to it the serial interface
  gps = new TeseoLIV3F(&Serial1, 7, 13);
  //Initialize the device
  gps->init();
  AccGyr = new LSM6DSOSensor (&DEV_I2C);
  AccGyr->Enable_X();
  AccGyr->Enable_G();
  Acc2 = new LIS2DW12Sensor (&DEV_I2C);
  Acc2->Enable_X();
  Mag = new LIS2MDLSensor (&DEV_I2C);
  Mag->Enable();
  PressTemp = new LPS22HHSensor(&DEV_I2C);
  PressTemp->Enable();
  HumTemp = new HTS221Sensor (&DEV_I2C);
  HumTemp->Enable();
  Temp3 = new STTS751Sensor (&DEV_I2C);
  Temp3->Enable();
  LoRaWAN.begin(US915);
  // Helium SubBand
  LoRaWAN.setSubBand(2);
  // Disable Adaptive Data Rate
  LoRaWAN.setADR(false);
  // Set Data Rate 1 - Max Payload 53 Bytes
  LoRaWAN.setDataRate(3);
  // Device IDs and Key
  LoRaWAN.joinOTAA(appEui, appKey, devEui);
  Serial.println("Start...");
  time1=millis();
}

void loop() {

   // Repeat at least 20 times per second this command
   gps->update();
   
  if((millis() - time1) > 30000)
  {  
  MyJson="{";
  printValidInfo();
    // Read humidity and temperature.
  float humidity = 0, temperature = 0;
  HumTemp->GetHumidity(&humidity);
  HumTemp->GetTemperature(&temperature);

  MyJson += "\"Hum\":";
  MyJson += String(humidity);
  MyJson += ",";

  // Read pressure and temperature.
  float pressure = 0, temperature2 = 0;
  PressTemp->GetPressure(&pressure);
  PressTemp->GetTemperature(&temperature2);

  MyJson += "\"Press\":";
  MyJson += String(pressure);
  MyJson += ",";

  //Read temperature
  float temperature3 = 0;
  Temp3->GetTemperature(&temperature3);

  float temp = (temperature + temperature2 + temperature3)/3;

  MyJson += "\"Temp\":";
  MyJson += String(temp);
  MyJson += ",";

  // Read accelerometer and gyroscope.
  int32_t accelerometer[3];
  int32_t gyroscope[3];
  AccGyr->Get_X_Axes(accelerometer);
  AccGyr->Get_G_Axes(gyroscope);

  MyJson += "\"Acc\":[";
  MyJson += String(accelerometer[0])+",";
  MyJson += String(accelerometer[1])+",";
  MyJson += String(accelerometer[2]);
  MyJson += "],";

  MyJson += "\"Gyro\":[";
  MyJson += String(gyroscope[0])+",";
  MyJson += String(gyroscope[1])+",";
  MyJson += String(gyroscope[2]);
  MyJson += "],";

  //Read accelerometer
  int32_t accelerometer2[3];
  Acc2->Get_X_Axes(accelerometer2);

  //Read magnetometer
  int32_t magnetometer[3];
  Mag->GetAxes(magnetometer);

  MyJson += "\"Mag\":[";
  MyJson += String(magnetometer[0])+",";
  MyJson += String(magnetometer[1])+",";
  MyJson += String(magnetometer[2]);
  MyJson += "]}";

  if (LoRaWAN.joined() && !LoRaWAN.busy())
    {   
        // Send Packet
        uint8_t payload[] ="";
        MyJson.getBytes(payload, sizeof(MyJson));
        LoRaWAN.sendPacket(1, payload, sizeof(payload));
        Serial.println("Data Sent");
    }
  time1=millis(); 
  Serial.println(MyJson);
  } 
}
void printValidInfo()
{
   data = gps->getData();
   if (data.gpgga_data.valid == 1)
   {
      int lat = (int) (data.gpgga_data.xyz.lat/100.0);
      int lat_mod = (int) (data.gpgga_data.xyz.lat) - (lat * 100);
      int lon = (int) (data.gpgga_data.xyz.lon/100.0);
      int lon_mod = (int) (data.gpgga_data.xyz.lon) - (lon * 100);
      char alt[7];
      char acc[5];
      dtostrf (data.gpgga_data.xyz.alt, 3, 2, alt);
      dtostrf (data.gpgga_data.acc, 4, 1, acc);
      snprintf(msg, MSG_SZ, "UTC:\t\t\t[ %02ld:%02ld:%02ld ]\r\n",
               data.gpgga_data.utc.hh,
               data.gpgga_data.utc.mm,
               data.gpgga_data.utc.ss);
      MyJson += "\"UTC\":[";
      MyJson += String(data.gpgga_data.utc.hh)+",";
      MyJson += String(data.gpgga_data.utc.mm)+",";
      MyJson += String(data.gpgga_data.utc.ss);
      MyJson += "],";

      snprintf(msg, MSG_SZ, "Latitude:\t\t[ %.02d' %.02d'' %c ]\r\n",
               lat,
               lat_mod,
               data.gpgga_data.xyz.ns);
      MyJson += "\"Lat\":";
      MyJson += String(lat)+"."+String(lat_mod);
      MyJson += ",";

      snprintf(msg, MSG_SZ, "Longitude:\t\t[ %.02d' %.02d'' %c ]\r\n",
               lon,
               lon_mod,
               data.gpgga_data.xyz.ew);
      MyJson += "\"Lon\":";
      MyJson += String(lon)+"."+String(lon_mod);
      MyJson += ",";
    
      snprintf(msg, MSG_SZ, "Satellites locked:\t[ %ld ]\r\n",
               data.gpgga_data.sats);

      snprintf(msg, MSG_SZ, "Position accuracy:\t[ %s ]\r\n",
               acc);

      snprintf(msg, MSG_SZ, "Altitude:\t\t[ %s%c ]\r\n",
               alt,
               (data.gpgga_data.xyz.mis + 32U));
      MyJson += "\"Alt\":";
      MyJson += String(alt);
      MyJson += ",";

      snprintf(msg, MSG_SZ, "Geoid infos:\t\t[ %ld%c ]\r\n",
               data.gpgga_data.geoid.height,
               data.gpgga_data.geoid.mis);

      snprintf(msg, MSG_SZ, "Diff update:\t\t[ %ld ]\r\n",
               data.gpgga_data.update);
   }
   else
   {
      Serial.print("Last position wasn't valid.\r\n\n");
   }
   Serial.print("\r\n\n");
}
