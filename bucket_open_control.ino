#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <VarSpeedServo.h>
#include <Bounce2.h>
#include <NewPing.h>

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#include "Wire.h"

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"

#include "MPU6050_6Axis_MotionApps20.h"
//#include "MPU6050.h" // not necessary if using MotionApps include file

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;

#define LED_PIN 13 // (Arduino is 13, Teensy is 11, Teensy++ is 6)
bool blinkState = false;


// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// Orientation/motion variables
Quaternion q;
VectorFloat gravity;
float euler[3];
float ypr[3];

// Use the following global variables and access functions to help store the overall
// rotation angle of the sensor
unsigned long last_read_time;
float         last_x_angle;  // These are the filtered angles
float         last_y_angle;
float         last_z_angle;  
float         last_gyro_x_angle;  // Store the gyro angles to compare drift
float         last_gyro_y_angle;
float         last_gyro_z_angle;

void set_last_read_angle_data(unsigned long time, float x, float y, float z, float x_gyro, float y_gyro, float z_gyro) {
  last_read_time = time;
  last_x_angle = x;
  last_y_angle = y;
  last_z_angle = z;
  last_gyro_x_angle = x_gyro;
  last_gyro_y_angle = y_gyro;
  last_gyro_z_angle = z_gyro;
}

//  Use the following global variables 
//  to calibrate the gyroscope sensor and accelerometer readings
float    base_x_gyro = 0;
float    base_y_gyro = 0;
float    base_z_gyro = 0;
float    base_x_accel = 0;
float    base_y_accel = 0;
float    base_z_accel = 0;


// This global variable tells us how to scale gyroscope data
float    GYRO_FACTOR;

// This global varible tells how to scale acclerometer data
float    ACCEL_FACTOR;

// Variables to store the values from the sensor readings
int16_t ax, ay, az;
int16_t gx, gy, gz;

// Buffer for data output
char dataOut[256];
// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}

// ================================================================
// ===                CALIBRATION_ROUTINE                       ===
// ================================================================
// Simple calibration - just average first few readings to subtract
// from the later data
void calibrate_sensors() {
  int       num_readings = 10;

  // Discard the first reading (don't know if this is needed or
  // not, however, it won't hurt.)
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
  // Read and average the raw values
  for (int i = 0; i < num_readings; i++) {
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    base_x_gyro += gx;
    base_y_gyro += gy;
    base_z_gyro += gz;
    base_x_accel += ax;
    base_y_accel += ay;
    base_y_accel += az;
  }
  
  base_x_gyro /= num_readings;
  base_y_gyro /= num_readings;
  base_z_gyro /= num_readings;
  base_x_accel /= num_readings;
  base_y_accel /= num_readings;
  base_z_accel /= num_readings;
}

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

// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup () {
   // join I2C bus (I2Cdev library doesn't do this automatically)
   Wire.begin();
   
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
   delay(500);
   myservo.detach();
   
   // initialize serial communication
   Serial.begin(57600);
   while (!Serial); // wait for Leonardo enumeration, others continue immediately

   // initialize device
   Serial.println(F("Initializing I2C devices..."));
   mpu.initialize();

   // verify connection
   Serial.println(F("Testing device connections..."));
   Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

   // load and configure the DMP
   Serial.println(F("Initializing DMP..."));
   devStatus = mpu.dmpInitialize();

    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        Serial.println(F("Enabling DMP..."));
        mpu.setDMPEnabled(true);

        // enable Arduino interrupt detection
        Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
        attachInterrupt(0, dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
        dmpReady = true;

        // Set the full scale range of the gyro
        uint8_t FS_SEL = 0;
        //mpu.setFullScaleGyroRange(FS_SEL);

        // get default full scale value of gyro - may have changed from default
        // function call returns values between 0 and 3
        uint8_t READ_FS_SEL = mpu.getFullScaleGyroRange();
        Serial.print("FS_SEL = ");
        Serial.println(READ_FS_SEL);
        GYRO_FACTOR = 131.0/(FS_SEL + 1);
        

        // get default full scale value of accelerometer - may not be default value.  
        // Accelerometer scale factor doesn't reall matter as it divides out
        uint8_t READ_AFS_SEL = mpu.getFullScaleAccelRange();
        Serial.print("AFS_SEL = ");
        Serial.println(READ_AFS_SEL);
        //ACCEL_FACTOR = 16384.0/(AFS_SEL + 1);
        
        // Set the full scale range of the accelerometer
        //uint8_t AFS_SEL = 0;
        //mpu.setFullScaleAccelRange(AFS_SEL);

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }

    // configure LED for output
    pinMode(LED_PIN, OUTPUT);
    
    // get calibration values for sensors
    calibrate_sensors();
    set_last_read_angle_data(millis(), 0, 0, 0, 0, 0, 0);

   debouncer.attach(PIN_BUTTON); // Привязываем объект антидребезга к порту кнопки открытия днища ковша
   debouncer.interval(25);
   delay(1500);
}

void loop() {

const float RADIANS_TO_DEGREES = 57.2958; //180/3.14159
 
    // if programming failed, don't try to do anything
    if (!dmpReady) return;
    
    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        // reset so we can continue cleanly
        mpu.resetFIFO();
        Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (mpuIntStatus & 0x02) {
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        
        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;
        
        // Obtain Euler angles from buffer
        //mpu.dmpGetQuaternion(&q, fifoBuffer);
        //mpu.dmpGetEuler(euler, &q);
        
        // Obtain YPR angles from buffer
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
  
       // Output complementary DMP data to the serial port.  The signs on the data needed to be
       // fudged to get the angle direction correct.       
       Serial.print("DMP:");
       Serial.print(ypr[2]*RADIANS_TO_DEGREES, 2);
       Serial.print(":");
       Serial.print(-ypr[1]*RADIANS_TO_DEGREES, 2);
       Serial.print(":");
       Serial.println(ypr[0]*RADIANS_TO_DEGREES, 2);
       

        // blink LED to indicate activity
        blinkState = !blinkState;
        digitalWrite(LED_PIN, blinkState);
    }
  
   // Измерение расстояния   
   if (millis() - sonar_timing > distacne_check_INTERVAL)
   {
      sonar_timing = millis();       
      distance = sonar.ping_cm();
      Serial.print("Ping: ");
      Serial.print(distance); // Send ping, get distance in cm and print result (0 = outside set distance range)
      Serial.println("cm");      
   }   
   
   // Контроль хода модели экскаватора
   if (distance > 44 && distance != 0)
    {digitalWrite(drive_backward_relay_PIN, LOW);}
    else {digitalWrite(drive_backward_relay_PIN, HIGH);} // Отключение хода назад в случае выхода расстояния за рамки рабочего диапазона
   if (distance < 84 && distance != 0)
    {digitalWrite(drive_forward_relay_PIN, LOW);}
    else {digitalWrite(drive_forward_relay_PIN, HIGH);} // Отключение хода вперед в случае выхода расстояния за рамки рабочего диапазона
    
   // Работа мотора открытия днища ковша в случае нажатия на кнопку
   debouncer.update();
   if (debouncer.fell() == true && butt_flag == false) // Проверка нажатия кнопки
   {
      butt_flag = true;
      myservo.attach(servoPin);
      myservo.write(180, 20, false); // Опускаем качельку вниз со скоростью 20
   }
   if (myservo.read() == 180 && butt_flag == true)
   {           
      servo_flag = true;
      myservo.write(0, 20, false); // Поднимаем качельку в исходное положение со скоростью 20
   }
   if (myservo.read() == 0 && servo_flag == true)
   {           
      myservo.detach();
      servo_flag = false;      
      butt_flag = false;             
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
