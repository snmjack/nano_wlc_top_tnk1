//************************************************************
// Water Level Controller V 1.0
//
// Over Head tank Serial Emitter
//************************************************************

// Necessary Libraries
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//#include <SoftwareSerial.h>

// Define Trig and Echo pin:
#define TopSensor 2
#define BottomSensor 3
#define echoPin 4
#define trigPin 5
#define flowpin 6

// Define variables:
long duration;
int distance;
int tankEmpty = 90;
int topsensor_stat = 1;
int bottomsensor_stat = 1;
int tanklevel = 0;


const size_t capacity = JSON_OBJECT_SIZE(5);
DynamicJsonDocument jsonsend(capacity);

LiquidCrystal_I2C lcd(0x27, 16, 2);
//SoftwareSerial linkSerial(10, 9); // RX, TX

// Setup
void setup() {
  // Begin Serial communication at a baudrate of 9600:
  Serial.begin(4800);
  //linkSerial.begin(9600);
  lcd.init();
  // Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(TopSensor, INPUT);
  pinMode(BottomSensor, INPUT);
  digitalWrite(TopSensor, LOW);
  digitalWrite(BottomSensor, LOW);
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Swapnil Mahajan");
  lcd.setCursor(0,1);
  lcd.print("WTR LVL CNTRLR ");
  delay(2000);
  lcd.clear();
}

//Loop
void loop() {
  //Read ultrasonic
  ultrasonicRead();
  topsensor_stat = digitalRead(TopSensor);
  bottomsensor_stat = digitalRead(BottomSensor);
  tanklevel = tankEmpty - distance;  
  
  jsonsend["topsensor_status"] = topsensor_stat;
  jsonsend["bottomsensor_status"] = bottomsensor_stat;
  jsonsend["overheadlevel"] = tanklevel;  
  //Send JSON serially
  
  
  sendMessage();
  
  lcd.setCursor(0,0);
  lcd.print("TNK1,");
  lcd.setCursor(6,0);
  lcd.print("TS=");
  lcd.setCursor(9,0);
  lcd.print(topsensor_stat);
  lcd.setCursor(11,0);
  lcd.print("BS=");
  lcd.setCursor(14,0);
  lcd.print(bottomsensor_stat);
  lcd.setCursor(0,1);
  lcd.print("Dist (cm)= ");
  lcd.setCursor(14,1);
  lcd.print(tanklevel);
}

//Functions
void ultrasonicRead(){
  digitalWrite(echoPin, LOW);   // set the echo pin LOW
  digitalWrite(trigPin, LOW);   // set the trigger pin LOW
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);  // set the trigger pin HIGH for 10μs
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);  // measure the echo time (μs)
  distance = duration*0.034/2;   // convert echo time to distance (cm)
}

void sendMessage() {
  if(!jsonsend.isNull()) {
    //serializeJson(jsonsend, linkSerial);
    serializeJson(jsonsend, Serial);
    //delay(500);
  }
  jsonsend.clear();
}
