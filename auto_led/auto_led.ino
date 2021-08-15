int PIR_sensor = A0;    //指定PIR模拟端口 A2
int peopleLED = 7;
bool haspeople = true;
void setup() {
  Serial.begin(9600);
  Serial.print("Asimov's LED!\n");
  pinMode(PIR_sensor, INPUT);   //设置PIR模拟端口为输入模式
  pinMode(peopleLED, OUTPUT);         //设置端口为输出模式
  digitalWrite(peopleLED, LOW);
}

void loop() {

  autoDectedPeople();

}

void autoDectedPeople()
{
  if (detectPeople())
  {
    Serial.println("感应到有人");
    if (haspeople)
    {
      digitalWrite(peopleLED, HIGH); //感应到有人
      delay(3000);
      digitalWrite(peopleLED, LOW);
      haspeople = false;
    }
  }
  else
  {
    haspeople = true;
    digitalWrite(peopleLED, LOW); //无人
    Serial.println("无人");
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
