#include <SoftwareSerial.h>
#include <Servo.h> //引入lib
SoftwareSerial mySerial(10, 11); // RX, TX
int port0 = A0;//连接光线传感器
int value0 = 0;//测量的模拟数值
int port1 = A1;
int value1 = 0;
int port2 = A2;
int value2 = 0;

String cmd1 = "AT+RST\r";

//String cmd2 = "AT+CWJAP=\"mq\",\"12345678\"\r";
String cmd2 = "AT+CWJAP=\"TP-LINK_9604\",\"m913919m\"\r";
String cmd3 = "AT+CIPMODE=1\r";
String cmd4 = "AT+CIPMUX=0\r";
String cmd5 = "AT+CIPSTART=\"TCP\",\"192.168.0.110\",10086\r";
String cmd6 = "AT+CIPSEND\r";

void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  mySerial.begin(115200);
  delay(1000);

  Serial.println("Start connecting");


  //等待光纤接入
  delay(3000);
//  setWifi(cmd1);
//  setWifi(cmd2);
//
//  setWifi(cmd3);
//
//  setWifinoDelay(cmd4);

  setWifinoDelay(cmd5);
  setWifinoDelay(cmd3);
  setWifinoDelay(cmd6);

  Serial.println("connecting done");
  mySerial.println("CONN_9527");

}
void loop()
{

  delay(1000);//间隔1秒读取数据
  readvalue();
  // printvalue();
  delay(100);
  sendvalue();

}

void setWifi(String cmd)
{
  Serial.println(cmd);
  mySerial.println(cmd);
  delay(5000);
}
void setWifinoDelay(String cmd)
{
  Serial.println(cmd);
  mySerial.println(cmd);
  delay(2000);
}
void reline()
{

  mySerial.println("CONN_9527");

}

void readvalue()
{
  value0 = analogRead(port0);
  delay(10);
  value1 = analogRead(port1);
  delay(10);
  value2 = analogRead(port2);
  delay(10);
}
void sendvalue()
{
  //返回服务器格式 axxbxxcxxd
  mySerial.print("a");
  mySerial.print(value0);
  mySerial.print("b");
  mySerial.print(value1);
  mySerial.print("c");
  mySerial.print(value2);
  mySerial.print("d");
}
void printvalue()
{
  Serial.print("value:  ");
  Serial.print(value0);//打印出来便于调试
  Serial.print("  ");
  Serial.print(value1);
  Serial.print("  ");
  Serial.println(value2);
}
