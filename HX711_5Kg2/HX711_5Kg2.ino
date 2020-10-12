
//叠被被
#include "HX711.h"
#include <SoftwareSerial.h>
  #include <Servo.h> //引入lib
float Weight = 0;
float Weight2 = 0;
 SoftwareSerial mySerial(10, 11); // RX, TX
      Servo servoleft;
      Servo servoright;
     int look=1;//开启称重
void setup()
{
	Init_Hx711();				//初始化HX711模块连接的IO设置
	Serial.begin(9600);
	Serial.print("Welcome to use!\n");
      while (!Serial) {
        ;
      }
      mySerial.begin(9600);
      delay(1000);
      Serial.println("Start connecting");
servoleft.attach(6);
servoright.attach(9);
	delay(1000);
	Get_Maopi();		//获取毛皮
}
/*
a:开始称重 look=1
A：暂停称重
B:初始化毛重

*/
void loop()
{

  if (mySerial.available())
      {
        String comdata = "";
        int i = 0;
        while (mySerial.available() > 0)
        {
          i++;
          char a = char(mySerial.read());
            comdata += a;
            delay(2);        
        }
        Serial.println(comdata);
        
       
if (comdata == "a" )
        {
look=1;
       
        }
     else if (comdata == "A" )
        {
look=0;
       
        } 
       else if (comdata == "B" )
        {
Get_Maopi();    //获取毛皮
       
        }   
      }
      
if(look)
{
   Weight = Get_Weight();  //计算放在传感器上的重物重量
 
 Serial.print("weight: ");
  Serial.print(float(Weight),0); //蓝牙串口显示重量 
  Serial.print(" g\n");  //显示单位
  delay(100);        //延时1s
  
 // mySerial.print("weigh:");
  mySerial.print(float(Weight),0); //串口显示重量  
  mySerial.print(" g\n");  //显示单位
  delay(500);
  }
}
