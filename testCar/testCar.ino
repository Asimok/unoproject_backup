int turnLeft = A0;
int turnRight = 13;
int front=5;
int back=6;

void setup() {

 //直流电机
   pinMode(turnLeft, OUTPUT);
  pinMode(turnRight, OUTPUT);
  pinMode(front, OUTPUT);
  pinMode(back, OUTPUT);
//  digitalWrite(turnLeft, HIGH);
//  digitalWrite(turnRight, LOW);
}

void loop() {
//  //forward 向前转
//  digitalWrite(input1, HIGH); //给高电平
//  digitalWrite(input2, LOW); //给低电平
//  digitalWrite(input3, HIGH); //给高电平
//  digitalWrite(input4, LOW); //给低电平
//  delay(1000);   //延时1秒
//
//  //stop 停止
//  digitalWrite(input1, LOW);
//  digitalWrite(input2, LOW);
//  digitalWrite(input3, LOW);
//  digitalWrite(input4, LOW);
//  delay(500);  //延时0.5秒
//
//
//  //back 向后转
//  digitalWrite(input1, LOW);
//  digitalWrite(input2, HIGH);
//  digitalWrite(input3, LOW);
//  digitalWrite(input4, HIGH);
//  delay(1000);
//analogWrite(front, 200);
 digitalWrite(turnLeft, HIGH);
 delay(1000);
 
}

void carRun(int goDirection, int carSpeed )
{
  if (goDirection == 5 || goDirection == 6)
  {
    analogWrite(goDirection, carSpeed);
  }

  if (goDirection == 12 || goDirection == 13)
  {
    if (goDirection == 12)
    {
      digitalWrite(goDirection, LOW);
      digitalWrite(goDirection + 1, HIGH);
      delay(500);
      digitalWrite(goDirection, HIGH);
    }
    else if (goDirection == 13)
    {
      digitalWrite(goDirection, LOW);
      digitalWrite(goDirection - 1, HIGH);
      delay(500);
      digitalWrite(goDirection, HIGH);
    }
  }
}
