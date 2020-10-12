int PIR_sensor = A0;    //指定PIR模拟端口 A5
int val = 0;            //存储获取到的PIR数值
 
void setup()
{
  pinMode(PIR_sensor, INPUT);   //设置PIR模拟端口为输入模式
  Serial.begin(9600);          //设置串口波特率为9600
}
 
void loop()
{
  delay(1000);
  
  val = analogRead(PIR_sensor);    //读取A0口的电压值并赋值到val
  if(val<1023)
  Serial.println(val);            //串口发送val值
 
}
