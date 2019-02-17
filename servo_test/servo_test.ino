#include <Servo.h>
class Sweeper
{
  Servo servo; // сервопривод
  int pos; // текущее положение сервы 
  int increment; // увеличиваем перемещение на каждом шаге
  int updateInterval; // промежуток времени между обновлениями
  unsigned long lastUpdate; // последнее обновление положения 
 
  public: 
  Sweeper(int interval)
  {
    updateInterval = interval;
    increment = 1;
  }
 
  void Attach(int pin)
  {
    servo.attach(pin);
  }
 
  void Detach()
  {
    servo.detach();
  }
 
  void Update()
  {
    if((millis() - lastUpdate) > updateInterval) // время обновлять
    {
      lastUpdate = millis();
      pos += increment;
      servo.write(pos);
      Serial.println(pos);
      if ((pos >= 180) || (pos <= 0)) // конец вращения
      {
        // обратное направление
        increment = -increment;
      }
   }
 }
};

Sweeper sweeper1(150); //аргумент - задержка в мс

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  sweeper1.Attach(9);
}

void loop() {
  // put your main code here, to run repeatedly:
  sweeper1.Update();
}
