/*
  uno r3 引脚定义

  直流电机驱动板：5 6 10 11 //一个当作两个用
  蜂鸣器 5v：12
  超声波传感器 5v：echo - A0 A1
  led 9

  软串口：3, 4



  //TODO
  舵机(放在esp8266)
  喷洒药水(放在esp8266)
  喇叭

  ESP8266 引脚定义
  DHT11 5v ：D7
  软串口：RX=D8,TX=D9
*/

#include <SoftwareSerial.h>
//#include <Servo.h>
//软串口
SoftwareSerial mySerial(3, 4); //RX=2,TX=4
/*系统配置*/
// 超声波避触发距离
float MAX_STOP = 15;
//避障开关
bool BARRIER_STATUS = false;
bool BLINK_BLOK = false;
//中断触发时机
int INTERUPT_TIME = 1;
//车速设置
int speed1 = 100;
int speed2 = 140;
int speed3 = 170;
int CAR_SPEED = speed1;
int innerSpeed = 105;
//直流电机
int left_back = 11;  //in4
int left_front = 10; //in3
int right_back = 6;  //in2
int right_front = 5; //in1
//蜂鸣器 低电平触发
int beep = 12;
//超声波测距
unsigned int EchoPin = A0;
unsigned int TrigPin = A1;
//舵机
//Servo myservo; //9
//led 
int LED =9;
//中断函数
int warning = 2;

//超声波传感器
float getDistance()
{
  unsigned long Time_Echo_us = 0;
  unsigned long Len_mm_X100  = 0;
  unsigned long Len_Integer = 0;
  unsigned int Len_Fraction = 0;

  digitalWrite(TrigPin, HIGH);
  delayMicroseconds(50);
  digitalWrite(TrigPin, LOW);
  Time_Echo_us = pulseIn(EchoPin, HIGH);
  if ((Time_Echo_us < 60000) && (Time_Echo_us > 1))
  {
    Len_mm_X100 = (Time_Echo_us * 34) / 20;
    Len_Integer = Len_mm_X100 / 100;
    Len_Fraction = Len_mm_X100 % 100;
    return Len_Integer + Len_Fraction / 100.0;
  }
}

//控制小车运动
void carRun(int goDirection, int carSpeed )
{
  // 判断引脚
  if (goDirection == 5 || goDirection == 6 || goDirection == 10 || goDirection == 11)
  {
    analogWrite(goDirection, carSpeed);
  }
}
//摇摆
void swag()
{
  //左转
  carRun(right_front, 0);
  carRun(left_back, 0);
  carRun(left_front, innerSpeed - 30);
  carRun(right_back, innerSpeed - 30);
  delay(200);
  //右转
  carRun(left_front, 0);
  carRun(right_back, 0);
  carRun(right_front, innerSpeed - 30);
  carRun(left_back, innerSpeed - 30);
  delay(400);
  //左转
  carRun(right_front, 0);
  carRun(left_back, 0);
  carRun(left_front, innerSpeed - 30);
  carRun(right_back, innerSpeed - 30);
  delay(230);
  carStop();
}
//刹车
void carStop()
{

  carRun(left_front, 0);
  carRun(left_back, 0);
  carRun(right_front, 0);
  carRun(right_back, 0);
  delay(10);
}
//蜂鸣器
void openBeep(int times)
{
  for (int i = 0; i < times; i++)
  {
    digitalWrite(beep, LOW);
    delay(100);
    digitalWrite(beep, HIGH);
    delay(100);
  }
}

//中断函数
void blink() {
  if (INTERUPT_TIME > 1)
  { Serial.println("进入中断");
    BLINK_BLOK = true;
    //后退
    carRun(right_front, 0);
    carRun(left_front, 0);
    carRun(right_back, CAR_SPEED);
    carRun(left_back, CAR_SPEED );
    for (int i = 0; i < 10; i++)
      delayMicroseconds(10000);
    carStop();
    //警报
    //    for (int i = 0; i < 7; i++)
    //    {
    //      digitalWrite(beep, LOW);
    //      for (int i = 0; i < 5; i++)
    //        delayMicroseconds(10000);
    //      digitalWrite(beep, HIGH);
    //      for (int i = 0; i < 5; i++)
    //        delayMicroseconds(10000);
    //    }
    //后退
    carRun(right_front, 0);
    carRun(left_front, 0);
    carRun(right_back, speed2);
    carRun(left_back, speed2);
    for (int i = 0; i < 50; i++)
      delayMicroseconds(10000);
    carStop();
    for (int i = 0; i < 5; i++)
      delayMicroseconds(10000);
    //转体180
    carRun(right_front, innerSpeed );
    carRun(left_back, innerSpeed);
    carRun(left_front, 0);
    carRun(right_back, 0);
    for (int i = 0; i < 120; i++)
      delayMicroseconds(10000);

    carStop();
    for (int i = 0; i < 10; i++)
      delayMicroseconds(10000);
    //跳出中断
    digitalWrite(warning, HIGH);
    BLINK_BLOK = false;
    Serial.println("跳出中断");

  }
  else
  {
    digitalWrite(beep, LOW);
    for (int i = 0; i < 5; i++)
      delayMicroseconds(10000);
    digitalWrite(beep, HIGH);
    for (int i = 0; i < 5; i++)
      delayMicroseconds(10000);
    Serial.println("避免首次中断！");
    BLINK_BLOK = false;
  }

}

//执行指令
void doComand(String comdata)
{
  Serial.print("串口数据：");
  Serial.println(comdata);
  /*
    指令集
    方向：
    > 前：w
    > 后：s
    > 左：a
    > 右：d
    > 刹车：q
    蜂鸣器：
    > 设置（1声）：1
    > 消息（3声）：3
    初始化完成提示：
    消息（3声）：3
    车速:
    z x c

    舵机
    m:打开
    n:关闭

  */
  if  (comdata.indexOf("q") != -1 )
  {
    //刹车
    carRun(right_back, 0);
    carRun(left_back, 0);
    carRun(right_front, 0);
    carRun(left_front, 0);
  }
  else if (comdata.indexOf("w") != -1 )
  {
    Serial.print("执行指令：w");
    carStop();

    //前进
    carRun(right_back, 0);
    carRun(left_back, 0);
    carRun(right_front, CAR_SPEED);
    carRun(left_front, CAR_SPEED);
  }
  else  if (comdata.indexOf("s") != -1 )
  {
    //后退
    carStop();
    carRun(right_front, 0);
    carRun(left_front, 0);
    carRun(right_back, CAR_SPEED );
    carRun(left_back, CAR_SPEED);
  }
  else  if (comdata.indexOf("a") != -1 )
  {
    //左转
    if (CAR_SPEED == speed1)
    {
      carRun(right_front, 0);
      carRun(left_back, 0);
      carRun(left_front, CAR_SPEED - 27);
      carRun(right_back, CAR_SPEED - 27);
    }
    else if (CAR_SPEED == speed2)
    {
      carRun(right_front, 0);
      carRun(left_back, 0);
      carRun(left_front, CAR_SPEED - 36);
      carRun(right_back, CAR_SPEED - 36);
    }
    else
    {
      carRun(right_front, 0);
      carRun(left_back, 0);
      carRun(left_front, CAR_SPEED - 55);
      carRun(right_back, CAR_SPEED - 55);
    }
  }
  else  if (comdata.indexOf("d") != -1 )
  {

    //右转
    if (CAR_SPEED == speed1)
    {
      carRun(left_front, 0);
      carRun(right_back, 0);
      carRun(right_front, CAR_SPEED - 27);
      carRun(left_back, CAR_SPEED - 27);
    }
    else if (CAR_SPEED == speed2)
    {
      carRun(left_front, 0);
      carRun(right_back, 0);
      carRun(right_front, CAR_SPEED - 36);
      carRun(left_back, CAR_SPEED - 36);
    }
    else
    {
      carRun(left_front, 0);
      carRun(right_back, 0);
      carRun(right_front, CAR_SPEED - 55);
      carRun(left_back, CAR_SPEED - 55);
    }

  }
  else  if (comdata.indexOf("1") != -1 )
  {
    openBeep(1);
  }
  else  if (comdata.indexOf("3") != -1 )
  {
    openBeep(3);
  }

  else  if (comdata.indexOf("z") != -1 )
  {
    //刹车
    carRun(right_back, 0);
    carRun(left_back, 0);
    carRun(right_front, 0);
    carRun(left_front, 0);
    CAR_SPEED = speed1;
    Serial.println("速度更新----1");
    openBeep(1);
  }
  else  if (comdata.indexOf("x") != -1 )
  {
    //刹车
    carRun(right_back, 0);
    carRun(left_back, 0);
    carRun(right_front, 0);
    carRun(left_front, 0);
    CAR_SPEED = speed2;
    Serial.println("速度更新----2");
    openBeep(1);
  }
  else  if (comdata.indexOf("c") != -1)
  {
    //刹车
    carRun(right_back, 0);
    carRun(left_back, 0);
    carRun(right_front, 0);
    carRun(left_front, 0);
    CAR_SPEED = speed3;
    Serial.println("速度更新----3");
    openBeep(1);
  }
  else  if (comdata.indexOf("o") != -1)
  {
    //刹车
    BARRIER_STATUS = true;
    Serial.println("避障模式----开");
    openBeep(1);
  }
  else  if (comdata.indexOf("p") != -1)
  {
    //刹车
    BARRIER_STATUS = false;
    Serial.println("避障模式----关");
    openBeep(1);
  }

  else  if (comdata.indexOf("m") != -1)
  {
digitalWrite(LED, HIGH);
    Serial.println("打开led");
    openBeep(1);

  }

  else  if (comdata.indexOf("n") != -1)
  {
digitalWrite(LED, LOW);
    Serial.println("关闭led");
    openBeep(1);
   
  }
}

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  while (!Serial) {
    ;
  }
  while (!mySerial) {
    ;
  }
  Serial.println("系统启动中");

  //外部中断
  attachInterrupt(0, blink, LOW);//低电平触发中断0，调用blink函数
  pinMode(warning, OUTPUT);
  digitalWrite(warning, HIGH);

//  //舵机
//  myservo.attach(8);
//  myservo.write(20);
  
  //直流电机
  pinMode(left_front, OUTPUT);
  pinMode(left_back, OUTPUT);
  pinMode(right_front, OUTPUT);
  pinMode(right_back, OUTPUT);
  //打开喇叭
  pinMode(beep, OUTPUT);
  //超声波测距
  pinMode(EchoPin, INPUT);
  pinMode(TrigPin, OUTPUT);
//LED
  pinMode(LED, OUTPUT);         //设置端口为输出模式
digitalWrite(LED, LOW);

  swag();
  openBeep(1);
  Serial.print("Car is running!\n");
}

void loop() {
  INTERUPT_TIME++;
  //软串口数据监测
  if (mySerial.available())
  {
    String data = "";
    int i = 0;
    while (mySerial.available() > 0)
    {
      i++;
      char a = char(mySerial.read());
      data += a;
      delay(2);
    }
    doComand(data);
  }

  //触发中断
  if (BARRIER_STATUS & !BLINK_BLOK)
  {
    float dis = getDistance();
    //    Serial.println(dis);
    if ( dis < MAX_STOP)
    {
      //最短距离 8厘米 触发中断
      digitalWrite(warning, LOW);
    }
  }
             

}
