#include <SoftwareSerial.h>
#include "ArduinoJson-v6.15.2.h"
#include <DallasTemperature.h>
SoftwareSerial mySerial(2, 3); //RX=2,TX=
#define ONE_WIRE_BUS 4               //1-wire数据总线连接在IO4
OneWire oneWire(ONE_WIRE_BUS);       //声明
DallasTemperature sensors(&oneWire); //声明

void setup(void)
{
  Serial.begin(115200);
  Serial.println("");
  mySerial.begin(115200);
  sensors.begin(); //初始化总线
  sensors.setWaitForConversion(false); //设置为非阻塞模式
}

unsigned long previousMillis = 0; //毫秒时间记录
const long interval = 1000;       //时间间隔
char msg[500];
void loop(void)
{
  //以下段落相当于每秒读取前次温度，并发起新一次温度转换
  unsigned long currentMillis = millis();         //读取当前时间
  if (currentMillis - previousMillis >= interval) //如果和前次时间大于等于时间间隔
  {
    previousMillis = currentMillis; //更新时间记录

    float tempC = sensors.getTempCByIndex(0); //获取索引号0的传感器摄氏温度数据
    if (tempC != DEVICE_DISCONNECTED_C)       //如果获取到的温度正常
    {
      //发送数据
      StaticJsonDocument<200> doc;
      Serial.print("\n当前温度是： ");
      Serial.print(tempC);
      Serial.println(" ℃");
      doc["sensor"] = "temperature";
      doc["time"] = millis();
      doc["data"] = tempC;
      serializeJson(doc, msg);
      mySerial.println(msg);
    }

    Serial.println("发起温度转换");
    sensors.requestTemperatures(); //发起新的温度转换
  }

  delay(20);
  Serial.print(".");
  
}
