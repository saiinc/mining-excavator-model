#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <VarSpeedServo.h>
#include <Bounce2.h>
#include <NewPing.h>

#define PIN_BUTTON 8 // Кнопка открытия днища ковша
#define servoPin A0 // Мотор открытия днища ковша
#define TRIGGER_PIN 9  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 10  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 150 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm (Max 255cm now).
#define drive_backward_relay_PIN 11 // Реле движения назад
#define drive_forward_relay_PIN 12 // Реле движения вперед
#define distacne_check_INTERVAL 500 // Частота проверки расстояния в мс
#define lcd_display_INTERVAL 500 // Частота обновления экрана дисплея в мс

// Подключение дисплея
// Software SPI (slower updates, more flexible pin options):
// pin 3 - Serial clock out (SCLK)
// pin 4 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 6 - LCD chip select (CS)
// pin 7 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544( 3, 4, 5, 6, 7);
#define lcd_light_Pin A1

VarSpeedServo myservo;    // create servo object to control a servo 
Bounce debouncer = Bounce(); // Создаем объект антидребезга
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // Создаем объект датчика расстояния

boolean butt_flag = false;
boolean servo_flag = false;
unsigned long sonar_timing = 0;
unsigned long lcd_timing = 0;
byte distance; // Max 255cm.

void setup () {
   // Инициализация дисплея
   pinMode(lcd_light_Pin, OUTPUT);
   analogWrite(lcd_light_Pin, 250);
   display.begin();
   display.cp437(true);
   display.setContrast(60);
   display.display(); // show splashscreen       
   display.clearDisplay();
   display.setTextColor(BLACK);   
   display.setTextSize(2);   

   // Настройка входов-выходов ардуины
   pinMode(PIN_BUTTON, INPUT_PULLUP);  
   pinMode(drive_backward_relay_PIN, OUTPUT);
   pinMode(drive_forward_relay_PIN, OUTPUT);
   digitalWrite(drive_backward_relay_PIN, HIGH);
   digitalWrite(drive_forward_relay_PIN, HIGH);
   myservo.attach(servoPin);  // Привязываем объект мотора открытия днища ковша к порту
   myservo.write(0, 0, true); // Задаем исходное положение для мотора открытия днища ковша
   myservo.detach();
   Serial.begin(9600);
   debouncer.attach(PIN_BUTTON); // Привязываем объект антидребезга к порту кнопки открытия днища ковша
   debouncer.interval(25);
   delay(2000);
}

void loop() {
   // Измерение расстояния   
   if (millis() - sonar_timing > distacne_check_INTERVAL)
   {
      sonar_timing = millis();       
      distance = sonar.ping_cm();
      Serial.print("Ping: ");
      Serial.print(distance); // Send ping, get distance in cm and print result (0 = outside set distance range)
      Serial.println("cm");      
   }   
   
   // Отключение хода в случае выхода расстояния за рамки рабочего диапазона
   if (distance > 35 && distance != 0)
    {digitalWrite(drive_backward_relay_PIN, LOW);}
    else {digitalWrite(drive_backward_relay_PIN, HIGH);}
   if (distance < 90 && distance != 0)
    {digitalWrite(drive_forward_relay_PIN, LOW);}
    else {digitalWrite(drive_forward_relay_PIN, HIGH);}
    
   // Работа мотора открытия днища ковша в случае нажатия на кнопку
   debouncer.update();
   if (debouncer.fell() == true && butt_flag == false)
   {
      butt_flag = true;
      myservo.attach(servoPin);
      myservo.write(180, 20, false);            
   }
   if (myservo.read() == 180 && butt_flag == true)
   {           
      servo_flag = true;
      myservo.write(0, 20, false);                   
   }
   if (myservo.read() == 0 && servo_flag == true)
   {           
      servo_flag = false;      
      butt_flag = false;
      myservo.detach();       
   }   

   // Вывод информации на дисплей
   if (millis() - lcd_timing > lcd_display_INTERVAL)
   {
      lcd_timing = millis();      
      display.clearDisplay();   
      display.setCursor(0,0);       
      display.println(utf8rus("Дист:"));  
      display.setCursor(0,17); 
      display.print(distance);
      display.println(utf8rus(" см"));      
      if (digitalRead(drive_backward_relay_PIN) == HIGH || digitalRead(drive_forward_relay_PIN) == HIGH)
      {
        display.setCursor(0,33);   
        display.println(utf8rus("CТОП!"));
      }
      display.display();      
   }
}


/* Recode russian fonts from UTF-8 to Windows-1251 */
String utf8rus(String source)
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length(); i = 0;

  while (i < k) {
    n = source[i]; i++;

    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x30;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x70;
          break;
        }
      }
    }
    m[0] = n; target = target + String(m);
  }
return target;
}
