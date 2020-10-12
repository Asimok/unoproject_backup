#include "HX711.h"
#include <SoftwareSerial.h>
#include<Stepper.h>

/*
  look=1; 开始称重
 a;开启 look=1
  A:关闭 look=0
  B:初始化左
  b:初始化右
  D：顺时针旋转
  d:逆时针旋转
*/
float Weight = 0;
float Weight2 = 0;
SoftwareSerial mySerial(5, 6); // RX, TX
// 电机内部输出轴旋转一周步数
const int STEPS_PER_ROTOR_REV = 32;

//  减速比
const int GEAR_REDUCTION = 128;

/*
   转子旋转一周需要走32步。转子每旋转一周，电机输出轴只旋转1/64周。
   （电机内部配有多个减速齿轮,这些齿轮会的作用是让转子每旋转一周，
   输出轴只旋转1/64周。）
   因此电机输出轴旋转一周则需要转子走32X64=2048步，即以下常量定义。
*/

// 电机外部输出轴旋转一周步数 （2048）
const float STEPS_PER_OUT_REV = STEPS_PER_ROTOR_REV * GEAR_REDUCTION;

// 定义电机控制用变量

// 电机旋转步数
int StepsRequired;

// 建立步进电机对象
// 定义电机控制引脚以及电机基本信息。
// 电机控制引脚为 8,9,10,11
// 以上引脚依次连接在ULN2003 驱动板 In1, In2, In3, In4

Stepper steppermotor(STEPS_PER_ROTOR_REV, 8, 10, 9, 11);
int look = 0;
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
  Serial.println("Start");

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


    if (comdata == "a" )
    {
      look = 1;

    }
    else if (comdata == "A" )
    {
      look = 0;
    }
    else if (comdata == "B" )
    {
      Get_Maopi();  

    }
    else if (comdata == "b" )
    {
     Get_Maopi2(); 
    }
   else if (comdata == "D" )
    {
      // 快速逆时针旋转一圈
      StepsRequired  =  - 280;
      steppermotor.setSpeed(1000);
      steppermotor.step(StepsRequired);
      delay(1);
    }
    else if (comdata == "d" )
    {
      // 慢速顺时针旋转一圈
      StepsRequired  = 280;
      steppermotor.setSpeed(1000);
      steppermotor.step(StepsRequired);
      delay(1);
    }
  }

  if (look)
  {
    Weight = Get_Weight();  //计算放在传感器上的重物重量
    Weight2 = Get_Weight2();
//"l123fr456i"

    Serial.print(float(Weight ), 0); //串口显示重量
   Serial.print("     ");
//    Serial.print("r");
    Serial.print(float(Weight2),0);
//    Serial.print("i");
    Serial.print("\n");
//    delay(500);

    mySerial.print("l");
    mySerial.print(float(Weight),0); //串口显示重量
    mySerial.print("fr");
    mySerial.print(float(Weight2),0);
    mySerial.print("i\n");

    delay(1000);

  }
}
