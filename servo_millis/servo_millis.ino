#include <VarSpeedServo.h>

#include <Servo.h> //используем библиотеку для работы с сервоприводом
Servo servo1; //объявляем переменную servo типа Servo
int ledPin = 13;      
#define  INTERVAL  200UL 
#define PIN_BUTTON 3
unsigned long timing = millis();
boolean servo_flag = false;
boolean butt_flag = false;
boolean butt;
void setup() //процедура setup
{      
  pinMode(PIN_BUTTON, INPUT_PULLUP);  //привязываем кнопку к порту 3
  servo1.attach(10); //привязываем привод к порту 10
  servo1.write(0);
  servo1.detach();
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT); // устанавливаем порт, как выход
  
}

void loop() //процедура loop
{
  static unsigned long previousMillis = 0;
  if(millis() - previousMillis > INTERVAL) {
    // сохраняем время последнего переключения
    previousMillis = millis(); 
  }     
  if(millis() - previousMillis > 0*INTERVAL/6&&millis() - previousMillis < 1*INTERVAL/6) {
      digitalWrite(ledPin,HIGH);}
  if(millis() - previousMillis > 1*INTERVAL/6&&millis() - previousMillis < 2*INTERVAL/6) {
      digitalWrite(ledPin,LOW);}
 
  butt = (!digitalRead(3));
  if (butt == true && millis() - timing > 1500 && servo_flag == false ) //кнопка нажата
  {       
    servo1.attach(10);            
    servo1.write(180); 
    servo_flag = true;
    timing = millis();
    butt_flag = true;
  }
 // Serial.println(millis());  
  if(millis() - timing > 1500 && servo_flag == true)
  {    
    servo1.write(0); 
    servo_flag = false;    
    timing = millis();
  }
  if(millis() - timing > 1500 && servo_flag == false && butt_flag == true)
  {        
    servo1.detach();
    timing = millis();
    butt_flag = false;
  }  
}
