#include <Servo.h> //используем библиотеку для работы с сервоприводом
Servo servo1; //объявляем переменную servo типа Servo
boolean butt_flag = 0;
boolean butt;
int rot_spd = 20; //скорость поворота сервы
int ugol1;

void setup() //процедура setup
{    
  Serial.begin(9600);
  pinMode(3, INPUT_PULLUP);  //привязываем кнопку к порту 3
  pinMode(4, OUTPUT);
  servo1.attach(10); //привязываем привод к порту 10
  servo1.write(5);
  servo1.detach();
}

void loop() //процедура loop
{
  butt = !digitalRead(3); //считать текущее положение кнопки
  if (butt == true && butt_flag == false) //кнопка нажата
    {
      butt_flag = true;     
      tone(4, 450, 4115); 
      servo1.attach(10); //привязываем привод к порту 10
      for(ugol1 = 5; ugol1 <= 160; ugol1++) //медленно открываем днище ковша
      {        
        servo1.write(ugol1); 
        Serial.println(ugol1);
        delay(rot_spd);
      }      
      delay(rot_spd*4);
      for(ugol1 = 160; ugol1 >= 5; ugol1--) //возвращаемся в исходное положение
      {
        Serial.println(ugol1);
        servo1.write(ugol1); 
        delay(rot_spd);
      }            
      butt_flag = false;    
      servo1.detach(); //отвязываем серву
    }
}
