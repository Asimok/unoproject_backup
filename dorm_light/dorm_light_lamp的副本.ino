#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT//支持小爱同学
#include <Blinker.h>

#define GPIO 0  //定义继电器输入引脚为D7

char auth[] = "6c8e330dd4a2";
 char ssid[] = "女寝专用";
char pswd[] = "208208nb";

// 新建组件对象
BlinkerButton Button1("btn-abc");//注意：要和APP组件’数据键名’一致
 
// 按下BlinkerAPP按键即会执行该函数
void button1_callback(const String & state) {
 BLINKER_LOG("get button state: ", state);
 digitalWrite(GPIO, !digitalRead(GPIO));
 Blinker.vibrate();
}
 
//小爱电源类操作的回调函数:
//当小爱同学向设备发起控制, 设备端需要有对应控制处理函数 
void miotPowerState(const String & state)
{
 BLINKER_LOG("need set power state: ", state);
 if (state == BLINKER_CMD_ON) {
 digitalWrite(GPIO, LOW);
 BlinkerMIOT.powerState("on");
 BlinkerMIOT.print();
 }
 else if (state == BLINKER_CMD_OFF) {
 digitalWrite(GPIO, HIGH);
 BlinkerMIOT.powerState("off");
 BlinkerMIOT.print();
 }
}
 
void setup() {
 // 初始化串口，并开启调试信息，调试用可以删除
 Serial.begin(115200); 
 BLINKER_DEBUG.stream(Serial);
 // 初始化IO
 pinMode(GPIO, OUTPUT);
 digitalWrite(GPIO, HIGH);
 
 // 初始化blinker
 Blinker.begin(auth, ssid, pswd);
 Button1.attach(button1_callback);
 
 //小爱同学务必在回调函数中反馈该控制状态 
 BlinkerMIOT.attachPowerState(miotPowerState);//注册回调函数
}
void loop() {
 Blinker.run();

}
