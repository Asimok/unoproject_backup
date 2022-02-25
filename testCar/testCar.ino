int left_back = 11;  //in4
int left_front = 10; //in3
int right_back = 6;  //in2
int right_front = 5; //in1

void setup() {

  //直流电机
  pinMode(left_front, OUTPUT);
  pinMode(left_back, OUTPUT);
  pinMode(right_front, OUTPUT);
  pinMode(right_back, OUTPUT);

}

void loop() {
carRun(right_front, 200);
// delay(1000);
 
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
