#include <Servo.h> //используем библиотеку для работы с сервоприводом
#include <Bounce2.h>
#define PIN_BUTTON 3
Servo servo1; //объявляем переменную servo типа Servo
boolean butt_flag = false;
boolean butt;
boolean butt_old = false;
int rot_spd = 20; //скорость поворота сервы
int ugol1;
Bounce debouncer = Bounce();

void setup() //процедура setup
{    
  Serial.begin(9600);
  //pinMode(3, INPUT);  //привязываем кнопку к порту 3
  pinMode(PIN_BUTTON, INPUT_PULLUP);  //привязываем кнопку к порту 3
  debouncer.attach(PIN_BUTTON);
  debouncer.interval(25);
  pinMode(4, OUTPUT);
  servo1.attach(10); //привязываем привод к порту 10
  servo1.write(5);
  servo1.detach();
}

void loop() //процедура loop
{
  debouncer.update();
  Serial.print(!digitalRead(3));
  butt = debouncer.fell(); //считать текущее положение кнопки
  //if (butt != butt_old) { delay(50); }
  //butt = !digitalRead(3);
  if (butt == true && butt_flag == false) //кнопка нажата
    {
      butt_flag = true;     
      tone(4, 450, 4115); 
      servo1.attach(10); //привязываем привод к порту 10
      for(ugol1 = 5; ugol1 <= 160; ugol1++) //медленно открываем днище ковша
      {        
        servo1.write(ugol1); 
        Serial.println(ugol1);
        unsigned long current_Time = millis();
        while(millis()-rot_spd < current_Time);
      }      
      unsigned long current_Time = millis();
      while(millis()-rot_spd*10 < current_Time);
      for(ugol1 = 160; ugol1 >= 5; ugol1--) //возвращаемся в исходное положение
      {
        Serial.println(ugol1);
        servo1.write(ugol1); 
        unsigned long current_Time = millis();
        while(millis()-rot_spd < current_Time);
      }            
      butt_flag = false;    
      servo1.detach(); //отвязываем серву
    }
}
