#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED显示宽度（像素）
#define SCREEN_HEIGHT 32 // OLED显示高度（像素）

// 使用软件SPI连接的SSD1306显示器声明（默认情况下）：
#define OLED_MOSI   D7  //DI   ------ 
#define OLED_CLK    D5  //D0  -------
#define OLED_DC    D2  //DC  ------- 
#define OLED_CS    D8  //CS  ------- 
#define OLED_RESET D3  //RES -------
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);


void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = 内部产生3.3V的显示电压
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // 不要继续，一直循环
  }
//在屏幕上显示初始显示缓冲区内容库用Adafruit启动屏幕初始化。
  display.display();

  // 清空缓冲区
  display.clearDisplay();

  // 用白色绘制单个像素
  display.drawPixel(10, 10, WHITE);

  // 在屏幕上显示显示缓冲区。必须在绘制命令后调用display（），使其在屏幕上可见！
  display.display();
  delay(2000);
}


void loop() {
    testdrawstyles();
    delay(2000);
}


void testdrawstyles(void) {
  display.clearDisplay();

  display.setTextSize(1);             // 正常1:1像素比例
  display.setTextColor(WHITE);        // 绘制白色文本
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("$60F3,$8F93,$7684,$5B57"));

  display.setTextColor(BLACK, WHITE); // Draw 'inverse' text
  display.println(202059);

  display.setTextSize(2);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.print(F("0x")); display.println(0xDEADBEEF, HEX);

  display.display();
  delay(2000);
}
