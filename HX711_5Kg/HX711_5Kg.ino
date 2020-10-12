#include "HX711.h"
#include <SoftwareSerial.h>
  #include <Servo.h> //引入lib
float Weight = 0;
float Weight2 = 0;
 SoftwareSerial mySerial(10, 11); // RX, TX
      Servo servoleft;
      Servo servoright;
     int look=0;
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
  Get_Maopi2();    //获取毛皮
}

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
        
         if (comdata == "C" )
        {
      
          delay(100);
          servoright.attach(9);
            delay(100);
          servoright.writeMicroseconds(1800);  //控制舵机转动相应的角度。
         // servoleft.writeMicroseconds(1200);  //控制舵机转动相应的角度。
          delay(300);//延时100毫秒
        mySerial.print("openleft");
        }
         else if (comdata == "c" )
        {
   
          delay(100);
  servoright.detach();
//          servoleft.writeMicroseconds(1800);  //控制舵机转动相应的角度。
          delay(300);//延时100毫秒
          mySerial.print("closeleft");
       
        }
        else if (comdata == "D" )
        {

          delay(100);
          servoright.attach(9);
            delay(100);
          servoright.writeMicroseconds(1200);  //控制舵机转动相应的角度。
          delay(300);//延时100毫秒
//       mySerial.print("openright");
        }
         else if (comdata == "d" )
        {
          delay(100);
//          servoright.writeMicroseconds(1800);  //控制舵机转动相应的角度。
servoright.detach();
          delay(300);//延时100毫秒
//          mySerial.print("closeright");
       
        }
 else if (comdata == "a" )
        {
look=1;
       
        }
     else if (comdata == "A" )
        {
look=0;
       
        } 
      }
      
if(look)
{
   Weight = Get_Weight();  //计算放在传感器上的重物重量
 Weight2 = Get_Weight2();
 
 Serial.print("left: ");
  Serial.print(float(Weight/1000),3); //串口显示重量
  Serial.print(" kg   ");  //显示单位
   Serial.print("right: ");  
  Serial.print(float(Weight2/1000),3);  
  Serial.print(" kg\n");  //显示单位
  Serial.print("\n");   //显示单位
  delay(100);        //延时1s
  
//  mySerial.print("left:");
//  mySerial.print(float(Weight/1000),3); //串口显示重量
//  mySerial.print(" kg");  //显示单位
//   mySerial.print("right:");  
//  mySerial.print(float(Weight2/1000),3);  
//  mySerial.print(" kg\n");  //显示单位
//  delay(1000);
  }
}
