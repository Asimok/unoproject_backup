#include <SoftwareSerial.h>
int led_pin = 9;
//定义一个10字节的整型数据变量cmd作为命令，这里可以修改为不同的数字。此处设置为10是为了有更好的兼容性。
char cmd[10];
//判断收到的cmd是否有内容
bool valid_cmd = false;

void setup()
{
//定义连接led的引脚为输出信号
pinMode(led_pin, OUTPUT);
Serial.begin(115200);
}
void loop()
{
/*以下部分是串口信息处理过程*/
//定义一个整数型变量i
int i;
//如果串口收到有数据
if (Serial.available() > 0)
{
//变量i最大为10
for (i = 0; i < 10; i++)
{
//清空缓存，存入cmd变量，并以\0作为结束符
cmd[i] = '\0';
}
//此时i只能取前9位，第10位是结束符\0
for (i = 0; i < 9; i++)
{
//再次判断串口如果收到有数据，防止数据丢失
if (Serial.available() > 0)
{
//给变量cmd赋值，取串口收到的前9位字符
cmd[i] = Serial.read();
delay(1);
}
else
{
//如果串口数据超过9位，后面的字符直接忽略，跳到下一步
break;
}
}
/*以上串口信息处理结束*/

//得到最终变量cmd的有效值
valid_cmd = true;
}

//判断变量cmd的值，开始处理
if (valid_cmd)
{
//如果变量cmd的前2位的值是on
if (0 == strncmp(cmd, "on", 2))
{
//则连接led的引脚电压被置高5V，
digitalWrite(led_pin, HIGH);
//串口打印返回值ON，表示ON的操作执行成功
Serial.println("ON");
}
else if (0 == strncmp(cmd, "off", 3)) //否则如果变量cmd的前3位的值是off
{
//则连接继电器的引脚电压被置低0V，灯的电路被断开，灯灭
digitalWrite(led_pin, LOW);
//串口打印返回值F，表示OFF的操作执行成功
Serial.println("OFF");
}
else //如果以上两个条件都不成立，前2位不是ON，或者前3位不是OFF，即不正确的命令
{
//仅串口打印返回值X，表示指令错误。
Serial.println("X");
}
//到此，变量cmd的指令被处理完毕
valid_cmd = false;
}
//延迟10毫秒，返回loop主程序继续读取新的串口指令
delay(10);

}
