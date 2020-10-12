#include <SoftwareSerial.h>
#include "ArduinoJson-v6.15.2.h"
#include <DallasTemperature.h>
#include <Wire.h>
SoftwareSerial mySerial(2, 3); //RX=2,TX=3 ESP8266
SoftwareSerial mySerial2(12, 13); //RX=12,TX=13 GPS
const long temperature_interval = 18000;       //温度传感器采集时间间隔
const long gps_interval = 15000;       //GPS采集时间间隔
const long light_interval = 30000;    //光线传感器采集时间间隔
const long mpu_interval = 2000;    //mpu传感器采集时间间隔
/*
  温度传感器
*/
#define temperature_bus 4               //温度传感器
OneWire oneWire(temperature_bus);       //声明
DallasTemperature temperature_sensors(&oneWire); //声明
unsigned long previousMillis_temperature = 0; //毫秒时间记录

/*
  光线传感器
*/
unsigned long previousMillis_light = 0; //毫秒时间记录
int light_port0 = A0;//连接光线传感器

/*
  mpu6050加速度传感器
*/
long accelX, accelY, accelZ;      // 定义为全局变量，可直接在函数内部使用
float gForceX, gForceY, gForceZ;
long gyroX, gyroY, gyroZ;
float rotX, rotY, rotZ;
unsigned long previousMillis_mpu = 0; //毫秒时间记录
/*
  GPS+北斗传感器
*/
struct
{
  char GPS_Buffer[80];
  bool isGetData;   //是否获取到GPS数据
  bool isParseData; //是否解析完成
  char UTCTime[11];   //UTC时间
  char latitude[11];    //纬度
  char N_S[2];    //N/S
  char longitude[12];   //经度
  char E_W[2];    //E/W
  bool isUsefull;   //定位信息是否有效
  char altitude[11];    //海拔
} Save_Data;
const unsigned int gpsRxBufferLength = 600;
char gpsRxBuffer[gpsRxBufferLength];
unsigned int ii = 0;
unsigned long previousMillis_gps = 0; //毫秒时间记录



long lastMsg = 0;
int value = 0;
const char* clientId = "HELMET_1";

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  mySerial2.begin(9600);
  Serial.println("连接");
  Save_Data.isGetData = false;
  Save_Data.isParseData = false;
  Save_Data.isUsefull = false;
  temperature_sensors.begin(); //初始化总线
  temperature_sensors.setWaitForConversion(false); //设置为非阻塞模式
  Wire.begin();
  setupMPU();
}

void loop() {

  //  long now = millis();
  //  if (now - lastMsg > 10000) {
  //    //每2s发布一次信息
  //    lastMsg = now;
  //    ++value;
  //  }

  fun_gps_sensor();
  fun_temperature_sensor();
  fun_light_sensor();
  //    fun_mpu6050_sensor();


}



void fun_temperature_sensor() {
  //以下段落相当于每秒读取前次温度，并发起新一次温度转换
  char msg[100];
  unsigned long currentMillis = millis();         //读取当前时间
  if (currentMillis - previousMillis_temperature >= temperature_interval) //如果和前次时间大于等于时间间隔
  {
    previousMillis_temperature = currentMillis; //更新时间记录

    float tempC = temperature_sensors.getTempCByIndex(0); //获取索引号0的传感器摄氏温度数据
    if (tempC != DEVICE_DISCONNECTED_C)       //如果获取到的温度正常
    {
      //发送数据
      StaticJsonDocument<200> temperature_data;
      Serial.print("\n当前温度是： ");
      Serial.print(tempC);
      Serial.println(" ℃");
      temperature_data["sensor"] = "temperature";
      temperature_data["helmet_id"] = clientId;
      temperature_data["temperature"] = tempC;
      serializeJson(temperature_data, msg);
      Serial.println(msg);
      mySerial.println(msg);
    }
    Serial.println("重新发起温度转换");
    temperature_sensors.requestTemperatures(); //发起新的温度转换
  }
}

void fun_light_sensor() {

  char msg[100];
  unsigned long currentMillis = millis();         //读取当前时间
  int light_value0 ;//测量的模拟数值
  if (currentMillis - previousMillis_light >= light_interval) //如果和前次时间大于等于时间间隔
  {
    previousMillis_light = currentMillis; //更新时间记录

    light_value0 = analogRead(light_port0);

    //发送数据
    StaticJsonDocument<200> temperature_data;
    Serial.print("\n当前环境光是： ");
    Serial.println(light_value0);
    temperature_data["sensor"] = "light";
    temperature_data["helmet_id"] = clientId;
    temperature_data["light"] = light_value0;
    serializeJson(temperature_data, msg);
    Serial.println(msg);
    mySerial.println(msg);

    Serial.println("重新检测环境光");

  }
}
void  fun_gps_sensor()
{

  gpsRead();  //获取GPS数据
  parseGpsBuffer();//解析GPS数据
  unsigned long currentMillis = millis();         //读取当前时间
  if (currentMillis - previousMillis_gps >= gps_interval) //如果和前次时间大于等于时间间隔
  {
    previousMillis_gps = currentMillis; //更新时间记录
    printGpsBuffer();//输出解析后的数据

  }

}
void fun_mpu6050_sensor() {

  recordAccelRegisters();
  recordGyroRegisters();
    printData();

  ////////////////////////////////////////测试////////////////////////////////////////////
  char msg[100];
  unsigned long currentMillis = millis();         //读取当前时间
  if (currentMillis - previousMillis_mpu >= mpu_interval) //如果和前次时间大于等于时间间隔
  {
    previousMillis_mpu = currentMillis; //更新时间记录
    StaticJsonDocument<200> mpu_data;

    mpu_data["sensor"] = "mpu6050";
    mpu_data["helmet_id"] = clientId;
    mpu_data["warning"] = "0";
    serializeJson(mpu_data, msg);
    Serial.println(msg);
    mySerial.println(msg);

  }
  delay(100);
}
/*-------------------------------mpu6050----------------------------------*/
void setupMPU() {
  // REGISTER 0x6B/REGISTER 107:Power Management 1
  Wire.beginTransmission(0b1101000); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet Sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B/107 - Power Management (Sec. 4.30)
  Wire.write(0b00000000); //Setting SLEEP register to 0, using the internal 8 Mhz oscillator
  Wire.endTransmission();

  // REGISTER 0x1b/REGISTER 27:Gyroscope Configuration
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4)
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg./s (转化为rpm:250/360 * 60 = 41.67rpm) 最高可以转化为2000deg./s
  Wire.endTransmission();

  // REGISTER 0x1C/REGISTER 28:ACCELEROMETER CONFIGURATION
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5)
  Wire.write(0b00000000); //Setting the accel to +/- 2g（if choose +/- 16g，the value would be 0b00011000）
  Wire.endTransmission();
}

void recordAccelRegisters() {
  // REGISTER 0x3B~0x40/REGISTER 59~64
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000, 6); //Request Accel Registers (3B - 40)

  // 使用了左移<<和位运算|。Wire.read()一次读取1bytes，并在下一次调用时自动读取下一个地址的数据
  while (Wire.available() < 6); // Waiting for all the 6 bytes data to be sent from the slave machine （必须等待所有数据存储到缓冲区后才能读取）
  accelX = Wire.read() << 8 | Wire.read(); //Store first two bytes into accelX （自动存储为定义的long型值）
  accelY = Wire.read() << 8 | Wire.read(); //Store middle two bytes into accelY
  accelZ = Wire.read() << 8 | Wire.read(); //Store last two bytes into accelZ
  processAccelData();
}

void processAccelData() {
  gForceX = accelX / 16384.0;     //float = long / float
  gForceY = accelY / 16384.0;
  gForceZ = accelZ / 16384.0;
}

void recordGyroRegisters() {
  // REGISTER 0x43~0x48/REGISTER 67~72
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000, 6); //Request Gyro Registers (43 ~ 48)
  while (Wire.available() < 6);
  gyroX = Wire.read() << 8 | Wire.read(); //Store first two bytes into accelX
  gyroY = Wire.read() << 8 | Wire.read(); //Store middle two bytes into accelY
  gyroZ = Wire.read() << 8 | Wire.read(); //Store last two bytes into accelZ
  processGyroData();
}

void processGyroData() {
  rotX = gyroX / 131.0;
  rotY = gyroY / 131.0;
  rotZ = gyroZ / 131.0;
}

void printData() {
  Serial.print("Gyro (deg)");
  Serial.print(" X=");
  Serial.print(rotX);
  Serial.print(" Y=");
  Serial.print(rotY);
  Serial.print(" Z=");
  Serial.print(rotZ);
  Serial.print(" Accel (g)");
  Serial.print(" X=");
  Serial.print(gForceX);
  Serial.print(" Y=");
  Serial.print(gForceY);
  Serial.print(" Z=");
  Serial.println(gForceZ);
  delay(100);
}


/*--------------------gps---------------------*/
void errorLog(int num)
{
  Serial.print("ERROR");
  Serial.println(num);

}

void printGpsBuffer()
{
  if (Save_Data.isParseData)
  {
    Save_Data.isParseData = false;



    if (Save_Data.isUsefull)
    {
      Save_Data.isUsefull = false;


      //发送数据
      char msg[300];
      StaticJsonDocument<300> gps_data;
//      Serial.println("\n当前GPS信息是： ");
//      Serial.print("Save_Data.UTCTime = ");
//      Serial.println(Save_Data.UTCTime);
//      Serial.print("Save_Data.latitude = ");
//      Serial.println(Save_Data.latitude);
//      Serial.print("Save_Data.N_S = ");
//      Serial.println(Save_Data.N_S);
//      Serial.print("Save_Data.longitude = ");
//      Serial.println(Save_Data.longitude);
//      Serial.print("Save_Data.E_W = ");
//      Serial.println(Save_Data.E_W);
//      Serial.print("Save_Data.altitude = ");
//      Serial.println(Save_Data.altitude);

      gps_data["sensor"] = "gps";
      gps_data["helmet_id"] = clientId;
      gps_data["longitude"] = Save_Data.longitude;
      gps_data["latitude"] = Save_Data.latitude;
      gps_data["N_S"] = Save_Data.N_S;
      gps_data["E_W"] = Save_Data.E_W;
      gps_data["altitude"] = Save_Data.altitude;
      serializeJson(gps_data, msg);
      Serial.println(msg);
      mySerial.println(msg);
      Serial.println("重新获取GPS数据");


    }
    else
    {
      Serial.println("GPS DATA is not usefull!");
    }

  }
}

void parseGpsBuffer()
{
  char *subString;
  char *subStringNext;
  if (Save_Data.isGetData)
  {
    Save_Data.isGetData = false;
    // Serial.println("**************");
    // Serial.println(Save_Data.GPS_Buffer);


    for (int i = 0 ; i <= 9 ; i++)
    {
      if (i == 0)
      {
        if ((subString = strstr(Save_Data.GPS_Buffer, ",")) == NULL)
          errorLog(1);  //解析错误
      }
      else
      {
        subString++;
        if ((subStringNext = strstr(subString, ",")) != NULL)
        {
          char usefullBuffer[20];
          switch (i)
          {
            case 1: memcpy(Save_Data.UTCTime, subString, subStringNext - subString);; break; //获取UTC时间
            //            case 2:memcpy(usefullBuffer, subString, subStringNext - subString);Serial.println(subString);break; //获取UTC时间
            case 2: memcpy(Save_Data.latitude, subString, subStringNext - subString);; break; //获取纬度信息
            case 3: memcpy(Save_Data.N_S, subString, subStringNext - subString);; break; //获取N/S
            case 4: memcpy(Save_Data.longitude, subString, subStringNext - subString); break; //获取纬度信息
            case 5: memcpy(Save_Data.E_W, subString, subStringNext - subString); break; //获取E/W
            case 9: memcpy(Save_Data.altitude, subString, subStringNext - subString); break; //获取海拔

            default: break;
          }

          subString = subStringNext;
          Save_Data.isParseData = true;
          Save_Data.isUsefull = true;

          //          if (usefullBuffer[0] == 'A')
          //            Save_Data.isUsefull = true;
          //          else if (usefullBuffer[0] == 'V')
          //            Save_Data.isUsefull = false;

        }
        else
        {
          errorLog(2);  //解析错误
        }
      }

      delay(500);
    }
  }
}


void gpsRead() {
  while (mySerial2.available())
  {
    gpsRxBuffer[ii++] = mySerial2.read();
    if (ii == gpsRxBufferLength)clrGpsRxBuffer();
  }

  char* GPS_BufferHead;
  char* GPS_BufferTail;
  //  if ((GPS_BufferHead = strstr(gpsRxBuffer, "$GPRMC,")) != NULL || (GPS_BufferHead = strstr(gpsRxBuffer, "$GNRMC,")) != NULL )
  if ((GPS_BufferHead = strstr(gpsRxBuffer, "$GNGGA,")) != NULL || (GPS_BufferHead = strstr(gpsRxBuffer, "$GNGGA,")) != NULL )
  {
    if (((GPS_BufferTail = strstr(GPS_BufferHead, "\r\n")) != NULL) && (GPS_BufferTail > GPS_BufferHead))
    {
      memcpy(Save_Data.GPS_Buffer, GPS_BufferHead, GPS_BufferTail - GPS_BufferHead);
      Save_Data.isGetData = true;

      clrGpsRxBuffer();
    }
  }
}

void clrGpsRxBuffer(void)
{
  memset(gpsRxBuffer, 0, gpsRxBufferLength);      //清空
  ii = 0;
}
