#include <Servo.h> //используем библиотеку для работы с сервоприводом

Servo servo1; //объявляем переменную servo типа Servo
boolean butt_flag = 0;
boolean butt;
//boolean servo_flag;

void setup() //процедура setup
{
  servo1.attach(10); //привязываем привод к порту 10
  pinMode(3, INPUT_PULLUP);
}

void loop() //процедура loop
{
  butt = !digitalRead(3); //считать текущее положение кнопки
  if (butt == true && butt_flag == false) //кнопка нажата
    {
      butt_flag = true;
      servo1.write(1); //ставим вал под 1
      delay(2000); //ждем 2 секунды
      servo1.write(179); //ставим вал под 179
      delay(2000); //ждем 2 секунды
      butt_flag = false;
      //servo_flag = !servo_flag;
    }
/*
   if (butt == false && butt_flag == true) //кнопка отпущена
    {
      butt_flag = false;
    }
  
  servo1.write(1); //ставим вал под 1
  delay(2000); //ждем 2 секунды
  servo1.write(179); //ставим вал под 179
  delay(2000); //ждем 2 секунды
  */
}
