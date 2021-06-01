#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif


#define OLED_DI   D7  //DI   ------ 
#define OLED_D0    D5  //D0  -------
#define OLED_DC    D2  //DC  ------- 
#define OLED_CS    D8  //CS  ------- 
#define OLED_RESET D3  //RES -------

U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ OLED_D0, /* data=*/ OLED_DI, /* cs=*/ OLED_CS, /* dc=*/ OLED_DC, /* reset=*/ OLED_RESET);


void setup(void) {
  Serial.begin(115200);
  u8g2.begin();
  u8g2.enableUTF8Print();   // enable UTF8 support for the Arduino print() function
}
void mainPage() {
  //  室温 湿度
  u8g2.setFont(u8g2_font_unifont_t_chinese1);//9px
  u8g2.setFontDirection(0);
  u8g2.setCursor(0, 13);
  u8g2.print("感知:15°C");
  u8g2.setCursor(95, 13);
  u8g2.print("33%");

  //  时间
  u8g2.setFont(u8g2_font_luBS18_tf);//18px
  u8g2.setFontDirection(0);
  u8g2.setCursor(25, 12 + 18 + 5);
  u8g2.print("09:18");
  u8g2.setFont(u8g2_font_lubB08_tn);//9px
  u8g2.setFontDirection(0);
  u8g2.setCursor(35, 12 + 18 + 5 + 11);
  u8g2.print("2020-03-21");

  //网络天气
  u8g2.setFont(u8g2_font_unifont_t_chinese1);//14px
  u8g2.setCursor(0, 62);
  u8g2.print("大庆");
  u8g2.setCursor(45, 62);
  u8g2.print("多云");
  u8g2.setCursor(85, 62);
  u8g2.print("15°C");
}

void draw_msg(String s) {
  //绘制边框
  u8g2.clearBuffer();
  u8g2.drawFrame(0, 0, 128, 64);
  u8g2.setFont(u8g2_font_unifont_t_chinese1);
  u8g2.sendBuffer();

  int16_t len = s.length();
  int8_t chunk = 0;
  int8_t oneline_maxnum = 7 * 3;

  if (len % oneline_maxnum == 0)
    chunk = len / oneline_maxnum ;
  else
    chunk = len / oneline_maxnum + 1;


  for (int8_t i = 0; i < chunk; i++)
  {
    //截取文字 每行最多 21个字符
    String temp_srt = "";
    //该行完整
    if (i * oneline_maxnum + oneline_maxnum <= len )
    {
      temp_srt = sub_str(s, i * oneline_maxnum / 3, (i * oneline_maxnum + oneline_maxnum) / 3);
    }
    else
    {
      temp_srt = sub_str(s, i * oneline_maxnum / 3, len / 3);
    }

    Serial.println(temp_srt);
    int k = i % 3;
    Serial.println(k);
    if (k == 0)
      u8g2.clearBuffer();
    if (k == 2)
    {
      u8g2.setCursor(5, 20 + k * 20);
      u8g2.print(temp_srt);
      u8g2.sendBuffer();
      delay(2000);
      u8g2.clearBuffer();
    }

    u8g2.setCursor(5, 20 + k * 20);
    u8g2.print(temp_srt);
    u8g2.sendBuffer();

  }

}

String deal_str(String temp_text1, int num )
{

  char temp_text[100];
  strcpy(temp_text, temp_text1.c_str());

  char Set[100];
  int mp = 0;
  int iCut = 0;
  int i = 0;
  int len = strlen(temp_text);
  for (i = 0; i < len; i++)
  {

    if (temp_text[i] >> 7 & 1) //如果最高位右移&运算后==1
    {

      Set[mp++] = i + 3;
      i += 2; //中文字符占2个字节 i++ 避免下次循环又读取同一个字符的第2个字节
    }
    else
    {

      Set[mp++] = i + 1;
    }
  }

  /*
    关键在于Set[mp++] 存储结束字符的位置 ，循环结束后直接读取结束位置的字符设置为'\0'即可把
  */
  iCut = Set[num - 1];
  temp_text[iCut] = '\0';

  //  Serial.println(temp_text);
  return String(temp_text);
}


String sub_str(String buf , int8_t start_index , int8_t end_index)
{

  if (start_index == 0)
    return deal_str(buf, end_index);
  else

  {
    String start_str = deal_str(buf, start_index);
    String end_str = deal_str(buf, end_index);
    end_str.remove(0, start_str.length());
    return end_str;
  }
}

void oled_log(int x,int y, String msg)
{
  u8g2.clearBuffer();
  u8g2.setCursor(x, y);
  u8g2.print(msg);
  u8g2.sendBuffer();
}
void loop(void) {


  u8g2.clearBuffer();
  mainPage();
  u8g2.sendBuffer();
  delay(3000);

  String buf_str = "今天是个好日子呀，我的心情非常的好啊,努力建设建立社会主义国家";
  draw_msg(buf_str);

  delay(2000);
}
