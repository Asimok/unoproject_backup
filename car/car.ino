//舵机范围 60-85-110
#include <Servo.h>
#include <SoftwareSerial.h>
#define stopDistance 10
//速度
int carSpeed = 0;
//发送信息
bool sendFrontDistance = false;
bool sendHavePeople = false;
/*
  指令集
  A:打开车灯
  a:关闭车灯
  E:自动驾驶
  e:关闭自动驾驶
  C:检测周边
  c:关闭检测周边
  D:连接
  d:断开连接
  G:打开喇叭
  g:关闭喇叭
  F B L R 前后左右
*/
//外部中断
int warning = 2;
//喇叭
int beep = 8;
//车灯
bool stopCar = false;
int carLED = 4;
int carStopLED = A3;
int carLeftLED = 13;
int carRightLED = 12;
//人体红外传感
int PIR_sensor = A2;    //指定PIR模拟端口 A2
int peopleLED = 7;           //指定peopleLED端口 7
//超声波测距
unsigned int EchoPin = A0;
unsigned int TrigPin = A1;
unsigned long Time_Echo_us = 0;
unsigned long Len_mm_X100  = 0;
unsigned long Len_Integer = 0;
unsigned int Len_Fraction = 0;
float aroundDistance[11] = {0};
//直流电机
int turnLeft = 11;
int turnRight = 10;
int front = 5;
int back = 6;
//舵机  角度范围 60-85-110
Servo servo180;
//自动驾驶
bool isAutoDrive = false;
bool isrun = true;
int detectTime = 0;
//蓝牙串口
//SoftwareSerial Serial(11, 10); // RX, TX

void setup() {
  Serial.begin(9600);
  Serial.print("Asimov's Car!\n");
  while (!Serial) {
    ;
  }
  Serial.begin(9600);
  delay(100);
  Serial.println("系统启动中");
  //外部中断
  attachInterrupt(0, blink, LOW);//低电平触发中断0，调用blink函数
  pinMode(warning, OUTPUT);
  digitalWrite(warning, HIGH);
  //打开喇叭
  pinMode(beep, OUTPUT);
  digitalWrite(beep, LOW);
  delay(500);
  digitalWrite(beep, HIGH);
  //打开车灯
  pinMode(carLED, OUTPUT);
  pinMode(carStopLED, OUTPUT);
  pinMode(carLeftLED, OUTPUT);
  pinMode(carRightLED, OUTPUT);
  digitalWrite(carLED, HIGH);
  digitalWrite(carLeftLED, HIGH);
  digitalWrite(carRightLED, HIGH);
  //超声波测距
  pinMode(EchoPin, INPUT);
  pinMode(TrigPin, OUTPUT);
  //人体红外
  pinMode(PIR_sensor, INPUT);   //设置PIR模拟端口为输入模式
  pinMode(peopleLED, OUTPUT);         //设置端口为输出模式
  //舵机转动到中间位置
  servo180.attach(3);
  servo180.write(85);
  //直流电机
  pinMode(turnLeft, OUTPUT);
  pinMode(turnRight, OUTPUT);
  pinMode(front, OUTPUT);
  pinMode(back, OUTPUT);
  //关闭车灯
  digitalWrite(carStopLED, LOW);
  digitalWrite(carLeftLED, LOW);
  digitalWrite(carRightLED, LOW);
  Serial.println("系统启动成功");
}

void loop() {

  if (Serial.available())
  {
    String data = "";
    int i = 0;
    while (Serial.available() > 0)
    {
      i++;
      char a = char(Serial.read());
      data += a;
      delay(2);
    }
    Serial.println(data);
    doComand(data);

  }
//  if (getDistance() < stopDistance)
//  {
//    //最短距离 8厘米 触发中断
//    digitalWrite(warning, LOW);
//  }
  sendMessage();
  autoDrive();
  delay(1);
}
void blink() {
  //中断函数blink()
  Serial.println("进入中断");
  carRun(front, 0);
  carRun(back, 255);
  for (int i = 0; i < 40; i++)
    delayMicroseconds(10000);
  carRun(back, 0);
  for (int i = 0; i < 7; i++)
  {
    digitalWrite(carLED, LOW);
    digitalWrite(beep, LOW);
    digitalWrite(carStopLED, LOW);
    for (int i = 0; i < 5; i++)
      delayMicroseconds(10000);
    digitalWrite(carLED, HIGH);
    digitalWrite(beep, HIGH);
    digitalWrite(carStopLED, HIGH);
    for (int i = 0; i < 5; i++)
      delayMicroseconds(10000);
  }

  digitalWrite(carLED, HIGH);
  digitalWrite(warning, HIGH);
}
void detectFront(int angleLeft, int angleRight, int angelCenter, int speed)
{
  //  检测正前方
  servo180.write(85);
  delay(10);
  float Frontdis = getDistance();
  //Serial.println(Frontdis);
  if (Frontdis < 10)
  {
    //最短距离 8厘米 触发中断
    digitalWrite(warning, LOW);
  }
}
void sendMessage()
{
  if (sendFrontDistance)
  {
    servo180.detach();
    Serial.print(getDistance());
    Serial.print("cm");
    delay(10);
    //检测周围是否有人
    autoDectedPeople();
    delay(200);
    servo180.attach(3);
    servo180.write(85);
  }
}
void doComand(String comdata)
{
  //执行指令
  /*
    指令集
    A:打开车灯
    a:关闭车灯
    E:自动驾驶
    e:关闭自动驾驶
    C:检测周边
    c:关闭检测周边
    D:连接
    d:断开连接
    F B L R 前后左右
    Z:刹车
    z:回正方向
    H：打开左转向
    h:关闭左转向
    I:打开右转向
    i:关闭右转向
  */
  if (comdata == "A" )
  {
    digitalWrite(carLED, HIGH);
  }
  else  if (comdata == "a" )
  {
    digitalWrite(carLED, LOW);
  }
  if (comdata == "C" )
  {
    sendFrontDistance = true;
    sendHavePeople = true;
  }
  else  if (comdata == "c" )
  {
    sendFrontDistance = false;
    sendHavePeople = false;
  }
  if (comdata == "D")
  {
    detectDistance(140, 40, 85, 30);
    //    delay(100);
    //    for (int i = 0; i < 11; i++)
    //    {
    //      Serial.print(aroundDistance[i]);
    //      Serial.print("  ");
    //    }
    //    Serial.println("");
    delay(100);
    digitalWrite(carLED, HIGH);
  }
  else  if (comdata == "d" )
  {
    for (int i = 0; i < 3; i++)
    {
      carRun(front, 0);
      carRun(back, 0);
      digitalWrite(carLED, HIGH);
      delay(100);
      digitalWrite(carLED, LOW);
      delay(100);

    }

  }
  if (comdata.startsWith("F"))
  {
    //前进
    digitalWrite(carStopLED, LOW);
    carRun(back, 0);
    carRun(front, carSpeed);
  }
  if (comdata.startsWith("B"))
  {
    digitalWrite(carStopLED, HIGH);
    carRun(front, 0);
    carRun(back, carSpeed);
  }
  if (comdata == "Z")
  {
    if (!stopCar)
    {
      stopCar = !stopCar;
      carRun(front, 0);
      carRun(back, 0);
      digitalWrite(carStopLED, HIGH);
    }
    else
    {
      stopCar = !stopCar;
      carRun(front, 0);
      carRun(back, 0);
      digitalWrite(carStopLED, LOW);
    }
  }
  if (comdata == "z")
  {
    digitalWrite(carLeftLED, LOW);
    digitalWrite(carRightLED, LOW);
    carRun(turnLeft, 0);
    carRun(turnRight, 0);
  }
  if (comdata.startsWith("L") )
  { digitalWrite(carLeftLED, HIGH);
    digitalWrite(carRightLED, LOW);
    carRun(turnLeft, 255);
    carRun(turnRight, 0);
  }
  if (comdata.startsWith("R"))
  {
    digitalWrite(carLeftLED, LOW);
    digitalWrite(carRightLED, HIGH);
    carRun(turnLeft, 0);
    carRun(turnRight, 255);
  }
  if (comdata == "G" )
  {
    digitalWrite(beep, LOW);
  }
  else  if (comdata == "g" )
  {
    digitalWrite(beep, HIGH);
  }
  if (comdata == "H" )
  {
    digitalWrite(carLeftLED, HIGH);
    digitalWrite(carRightLED, LOW);
  }
  else  if (comdata == "h" )
  {
    digitalWrite(carLeftLED, LOW);
  }
  //自动驾驶
  if (comdata == "E" )
  {
    detectTime = 0;
    isrun = true;
    isAutoDrive = true;
    detachInterrupt(0);//关中断
    digitalWrite(beep, LOW);
    delay(300);
    digitalWrite(beep, HIGH);
    delay(300);
    digitalWrite(beep, LOW);
    delay(300);
    digitalWrite(beep, HIGH);
  }
  else  if (comdata == "e" )
  { detectTime = 0;
    isrun = true;
    isAutoDrive = false;
    //停车
    carRun(front, 0);
    carRun(back, 0);
    digitalWrite(carStopLED, HIGH);
    attachInterrupt(0, blink, LOW);//低电平触发中断0，调用blink函数

    digitalWrite(beep, LOW);
    delay(100);
    digitalWrite(beep, HIGH);
    delay(100);
    digitalWrite(beep, LOW);
    delay(100);
    digitalWrite(beep, HIGH);
  }
  if (comdata == "I" )
  {
    digitalWrite(carRightLED, HIGH);
    digitalWrite(carLeftLED, LOW);
  }
  else  if (comdata == "i" )
  {
    digitalWrite(carRightLED, LOW);
  }
  if (comdata == "1" )
  {
    carSpeed = 60;
  }
  else if (comdata == "2" )
  {
    carSpeed = 110;
  }
  else if (comdata == "3" )
  {
    carSpeed = 200;
  }
  else if (comdata == "0" )
  {
    carSpeed = 0;
  }
}
void detectDistance(int angleLeft, int angleRight, int angelCenter, int speed) {
  cleanDistance();
  for (int i = angelCenter; i <= angleLeft; i += 5)
  {
    //左转
    servo180.write(i);
    delay(10);
    if (i >= 60 && i <= 110)
      writeAroundDistance(i, getDistance());
    delay(speed);
    if (i == angleLeft)
    {
      for (int i = angleLeft; i >= angleRight; i -= 5)
      {
        //右转
        servo180.write(i);
        delay(10);
        if (i >= 60 && i <= 110)
          writeAroundDistance(i, getDistance());
        delay(speed);
        if (i == angleRight)
        {
          for (int i = angleRight; i <= 85; i += 5)
          {
            //左转回归到正向
            servo180.write(i);
            delay(10);
            if (i >= 60 && i <= 110)
              writeAroundDistance(i, getDistance());
            delay(speed);
            if (i == angelCenter)
              break;
          }
        }
      }
    }
  }
}
void autoDectedPeople()
{
  if (detectPeople())
  {
    digitalWrite(peopleLED, HIGH); //感应到有人
    Serial.println("感应到有人");
  }
  else
  {
    digitalWrite(peopleLED, LOW); //无人
    Serial.println("无人");
  }
}
void writeAroundDistance(int i, float value)
{
  int num = (i - 60) / 5;
  if (aroundDistance[num] == 0)
  { aroundDistance[num] = value;
  }
  else if (aroundDistance[num] > value)
  { //选取距离最小值
    aroundDistance[num] = value;
  }
}
float getDistance()
{
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
bool detectPeople()
{
  int val = 0;            //存储获取到的PIR数值
  val = analogRead(PIR_sensor);    //读取A2口的电压值并赋值到val
  if (val > 150)//判断PIR数值是否大于150，
  {
    return true;
  }
  else
  {
    return false;
  }
}
void cleanDistance()
{
  for (int k = 0 ; k < 11  ; k++ )
  {
    aroundDistance[k] = 0;
  }
}
void print11Distance()
{
  for (int i = 0; i < 11; i++)
  {
    Serial.print(i * 5 + 60);
    Serial.print(":");
    Serial.print(aroundDistance[i]);
    Serial.print("   ");
    Serial.println("");
  }
}

void carRun(int goDirection, int carSpeed )
{
  if (goDirection == 5 || goDirection == 6 || goDirection == 10 || goDirection == 11)
  {
    analogWrite(goDirection, carSpeed);
  }
}
void autoCarRun()
{
  if (isrun)
  { digitalWrite(carStopLED, LOW);
    carRun(back, 0);
    carRun(front, 150);
  }
}
void carStop()
{
  digitalWrite(carStopLED, HIGH);
  carRun(back, 0);
  carRun(front, 0);
}
void autoDrive()
{

  if (isAutoDrive) {
    //自动化处理
    //自动驾驶 速度2挡
    //前进
    autoCarRun();

    if (getDistance() < stopDistance)
    {
      //检测到正前方障碍物 停车
      isrun = false;
      carStop();
      //检测周边可行路径
      int maxValue = aroundDistance[0];
      int runAngle = 60;
      while (  detectTime < 2)
      {

        detectDistance(110, 60, 85, 30);
        detectTime++;

        for (int i = 0; i < 11; i++)
        {
          //         Serial.print(aroundDistance[i]);
          //      Serial.print("  ");
          if (maxValue < aroundDistance[i])
          {
            maxValue = aroundDistance[i];
            runAngle = 5 * i + 60;

          }
        }
        //     Serial.println("");
        delay(100);


      }
      //60度最右
      if (runAngle >= 85)
      {
        servo180.write(runAngle);
        Serial.println(runAngle);
        //左边没有障碍物
        //1.右后方倒车
        digitalWrite(carLeftLED, LOW);
        digitalWrite(carRightLED, HIGH);
        carRun(turnLeft, 255);
        carRun(turnRight, 0);
        delay(100);
        carRun(turnLeft, 0);
        carRun(turnRight, 255);
        delay(10);
        digitalWrite(carStopLED, HIGH);
        carRun(front, 0);
        carRun(back, 255);
        delay(1000);
        //2.左转
        digitalWrite(carLeftLED, HIGH);
        digitalWrite(carRightLED, LOW);
        carRun(turnLeft, 0);
        carRun(turnRight, 255);
        delay(100);
        carRun(turnLeft, 255);
        carRun(turnRight, 0);
        carRun(front, 210);
        carRun(back, 0);
        delay(2000);
        //回正
        digitalWrite(carLeftLED, LOW);
    digitalWrite(carRightLED, LOW);
    carRun(turnLeft, 0);
    carRun(turnRight, 0);
    servo180.write(85);
      }
      else
      {
          servo180.write(runAngle);
        Serial.println(runAngle);
        //右边没有障碍物
        //1.左后方倒车
        digitalWrite(carLeftLED, HIGH);
        digitalWrite(carRightLED, LOW);
        carRun(turnLeft, 0);
        carRun(turnRight, 255);
        delay(100);
        carRun(turnLeft, 255);
        carRun(turnRight, 0);
        delay(10);
        digitalWrite(carStopLED, HIGH);
        carRun(front, 0);
        carRun(back, 255);
        delay(1000);
        //2.右转
        digitalWrite(carLeftLED, LOW);
        digitalWrite(carRightLED, HIGH);
        carRun(turnLeft, 255);
        carRun(turnRight, 0);
        delay(100);
        carRun(turnLeft, 0);
        carRun(turnRight, 255);
        carRun(front, 210);
        carRun(back, 0);
        delay(2000);
        //回正
        digitalWrite(carLeftLED, LOW);
    digitalWrite(carRightLED, LOW);
    carRun(turnLeft, 0);
    carRun(turnRight, 0);
    servo180.write(85);
      }
      detectTime = 0;
    }



  }
}
