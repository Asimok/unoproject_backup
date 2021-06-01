void setup() {
  Serial.begin(115200);

}
void draw_msg(String s) {
  String temp_srt = "";
  int16_t len = s.length();
  int8_t chunk = 0;
  int8_t oneline_maxnum = 7 * 3;


  if (len % oneline_maxnum == 0)
    chunk = len / oneline_maxnum ;
  else
    chunk = len / oneline_maxnum +1;


  for (int8_t i = 0; i < chunk; i++)
  {
    //截取文字 每行最多 21个字符
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

  String start_str = deal_str(buf, start_index);
  String end_str = deal_str(buf, end_index);
  end_str.remove(0, start_str.length());
  return end_str;
}

void loop() {
  

  String buf_str = "今天是个";
  draw_msg(buf_str);
//  Serial.println(sub_str(buf_str, 8, ));

  delay(1000);
}
