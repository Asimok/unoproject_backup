    #include <SoftwareSerial.h>
 #include<Stepper.h>
    SoftwareSerial mySerial(5, 6); // RX, TX
    // 电机内部输出轴旋转一周步数
const int STEPS_PER_ROTOR_REV = 32; 
 
//  减速比
const int GEAR_REDUCTION = 1;
 
/*
 * 转子旋转一周需要走32步。转子每旋转一周，电机输出轴只旋转1/64周。
 * （电机内部配有多个减速齿轮,这些齿轮会的作用是让转子每旋转一周，
 * 输出轴只旋转1/64周。）
 * 因此电机输出轴旋转一周则需要转子走32X64=2048步，即以下常量定义。
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
void setup() {
 Serial.begin(9600);
      while (!Serial) {
        ;
      }
      mySerial.begin(9600);
      delay(2000);
      Serial.println("Start connecting");

}

void loop() {
   if (mySerial.available())
      {
        String comdata = "";
        int i = 0;
        while (mySerial.available() > 0)
        {
          i++;
          char a = char(mySerial.read());
            comdata += a;
            delay(1);        
        }
        Serial.println(comdata);
        
  if (comdata == "D" )
        {
       // 快速逆时针旋转一圈
  StepsRequired  =  - 200;   
  steppermotor.setSpeed(1000);  
  steppermotor.step(StepsRequired);
  delay(1);
        }
         else if (comdata == "d" )
        {
              // 慢速顺时针旋转一圈
  StepsRequired  = 200; 
  steppermotor.setSpeed(1000);   
  steppermotor.step(StepsRequired);
  delay(1);
        }


        
      
      }
}
