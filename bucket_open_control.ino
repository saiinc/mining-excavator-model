
#include <VarSpeedServo.h>
#include <Bounce2.h>
#include <NewPing.h>

#define PIN_BUTTON 3
#define ledPin 13
#define TRIGGER_PIN 7  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 6  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define drive_back_relay_PIN 4
#define drive_front_relay_PIN 5
#define led_INTERVAL  500UL 
#define distacne_check_INTERVAL 500

VarSpeedServo myservo;    // create servo object to control a servo 
Bounce debouncer = Bounce();
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

boolean butt_flag = false;
boolean servo_flag = false;
unsigned long sonar_timing = 0;
int distance;

void setup () {
   pinMode(ledPin, OUTPUT); // устанавливаем порт, как выход
   myservo.attach(10);  // attaches the servo on pin 9 to the servo object
   pinMode(PIN_BUTTON, INPUT_PULLUP);  //привязываем кнопку к порту
   pinMode(drive_back_relay_PIN, OUTPUT);
   pinMode(drive_front_relay_PIN, OUTPUT);
   myservo.write(5, 0, true);
   myservo.detach();
   Serial.begin(9600);
   debouncer.attach(PIN_BUTTON);
   debouncer.interval(25);
}

void loop() {
   static unsigned long previousMillis = 0;
   if(millis() - previousMillis > led_INTERVAL) 
   {
    // сохраняем время последнего переключения
    previousMillis = millis(); 
   }     
   if(millis() - previousMillis > 0*led_INTERVAL/6 && millis() - previousMillis < 1*led_INTERVAL/6) {
      digitalWrite(ledPin,HIGH);}
   if(millis() - previousMillis > 1*led_INTERVAL/6 && millis() - previousMillis < 2*led_INTERVAL/6) {
      digitalWrite(ledPin,LOW);}
      
   if (millis() - sonar_timing > distacne_check_INTERVAL)
   {
      sonar_timing = millis(); 
      Serial.print("Ping: ");
      distance = sonar.ping_cm();
      Serial.print(distance); // Send ping, get distance in cm and print result (0 = outside set distance range)
      Serial.println("cm,   ");
   }   
   if (distance == 0)
   {
      digitalWrite(drive_back_relay_PIN, HIGH);
      digitalWrite(drive_front_relay_PIN, HIGH);
   }
    else
      {
        if (distance < 50)
          {digitalWrite(drive_back_relay_PIN, HIGH);}
          else {digitalWrite(drive_back_relay_PIN, LOW);}   
        if (distance > 110)
          {digitalWrite(drive_front_relay_PIN, HIGH);}
          else {digitalWrite(drive_front_relay_PIN, LOW);}   
      }   
      
   debouncer.update();
   if (debouncer.fell() == true && butt_flag == false)
   {
      butt_flag = true;
      myservo.attach(10);
      myservo.write(180, 20, false);      
      butt_flag = true;
   }
   if (myservo.read() == 180 && butt_flag == true)
   {           
      servo_flag = true;
      myservo.write(5, 20, false);                   
   }
   if (myservo.read() == 5 && servo_flag == true)
   {           
      servo_flag = false;      
      butt_flag = false;
      myservo.detach();       
   }   
}

