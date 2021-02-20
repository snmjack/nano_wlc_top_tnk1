//************************************************************
// Water Level Controller V 1.0
//
// Master LCD Program direct receiver
//************************************************************

// Necessary Libraries
//#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Necessary for OLED Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Define Relay Pin
#define manualOnPin 2
#define relayPin 3
#define ledpin 4

#define TopSensor 9
#define BottomSensor 10
#define echoPin 11
#define trigPin 12
//#define flowpin 6

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//const size_t capacity = JSON_OBJECT_SIZE(3) + 75;
//DynamicJsonDocument doc(capacity);

const unsigned char Motoricon [] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x7f, 0xff, 
  0xff, 0x80, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x7f, 
  0xcc, 0xff, 0xce, 0x7f, 0xcc, 0xff, 0xcf, 0x3f, 0xcc, 0x00, 0x0f, 0x3f, 0xcc, 0x00, 0x0f, 0x3f, 
  0xcc, 0xff, 0xcf, 0x3f, 0xcc, 0xff, 0xcf, 0x3f, 0xcc, 0x00, 0x0f, 0x03, 0xcc, 0x00, 0x0f, 0x03, 
  0xcc, 0xff, 0xcf, 0x3f, 0xcc, 0xff, 0xcf, 0x3f, 0xcc, 0x00, 0x0f, 0x3f, 0xcc, 0x00, 0x0f, 0x3f, 
  0xcc, 0xff, 0xcf, 0x3f, 0xcc, 0xff, 0xce, 0x7f, 0xc0, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x01, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x07, 0xff, 0xf8, 0x00, 0x07, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

int topsensor_status = 1;
int bottomsensor_status = 1;
int overheadlevel = 0;
int tankEmpty = 90;
int flag = 1;

long duration;
int distance;
int tanklevel = 0;

bool relay_status = false;
int relay_state = LOW;
char* motor_status;
char* flow_status;

int button_reading;           // the current reading from the input pin
int previous = HIGH;    // the previous reading from the input pin
long timeButton = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers

unsigned long previousTimeOled = millis();
long timeIntervalOled = 1;

unsigned long previousTimeRec = millis();
long timeIntervalRec = 1;

unsigned long previousTimeMotor = millis();
long timeIntervalMotor = 1;

unsigned long previousTimeManual = millis();
long timeIntervalManual = 1;


// Setup
void setup() {
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(TopSensor, INPUT);
  pinMode(BottomSensor, INPUT);
  digitalWrite(TopSensor, LOW);
  digitalWrite(BottomSensor, LOW);
  
  pinMode(ledpin, OUTPUT);
  digitalWrite(ledpin, LOW);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  pinMode(manualOnPin, INPUT);
  digitalWrite(manualOnPin, LOW);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);// initialize with the I2C addr 0x3D (for the 128x64)
  display.display();
  delay(2000);
  display.clearDisplay(); 
  displayboot();
  display.clearDisplay(); 
 }


 void loop() {
  
   unsigned long currentTime = millis();

   
   // Task 1 - Get Sensor Data
      if(currentTime - previousTimeRec > timeIntervalRec) {
         previousTimeRec = currentTime;
         ultrasonicRead();
         topsensor_status = digitalRead(TopSensor);
         bottomsensor_status = digitalRead(BottomSensor);
         overheadlevel = tankEmpty - distance;
         //overheadlevel = distance;  
         }
      
   // Task 3 - Draw Tank and display level
      if(currentTime - previousTimeOled > timeIntervalOled) {
         previousTimeOled = currentTime;
         display.clearDisplay();
         motorFunction();
         motorDisplayFunction();
         displayOhtLevel(SCREEN_HEIGHT, overheadlevel);
         display.display();
         }
      
      if(currentTime - previousTimeManual > timeIntervalManual) {
          previousTimeManual = currentTime;
          //Manual Trigger
          manualTrigger();
      }
  
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

void displayOhtLevel(int height, int ohtLevel) {
  int final_height = 64 - ((height*ohtLevel)/tankEmpty);
  if(final_height <= 10) {
    final_height = 10;
  }
  if(final_height > 64) {
    final_height = 64;
  }
  display.setTextSize(1);
  display.setCursor(71,0);
  display.print("Over Head");
  display.drawRect(69, 10, 59, 54, WHITE);
  display.fillRect(69, final_height, 59, 54, WHITE);
  }

void motorDisplayFunction() {
  //motor_status = "";
  if(relay_status  == true) {
    motor_status = "ON";
  } 
  if(relay_status  == false) {
    motor_status = "OFF";
  }
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.drawBitmap(0, 0,  Motoricon, 32, 32, WHITE);
  display.setCursor(34,10);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print(motor_status);

  display.setCursor(0,34);
  display.print("T");
  display.setCursor(16,34);
  display.print(topsensor_status);
  display.setCursor(0,50);
  display.print("B");
  display.setCursor(16,50);
  display.print(bottomsensor_status);
  display.setCursor(34,34);
  display.print("F");
  display.setCursor(50,34);
  display.print(flag);
}

void motorFunction() {
  
  display.setCursor(34,10);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  
  if(topsensor_status == 1 && bottomsensor_status == 1){
    relay_status = false;
    relay_state = LOW;
    flag = 1;
  } else {
  
  if(topsensor_status == 0 && bottomsensor_status == 0){
      relay_status = true;
      relay_state = HIGH;
      flag = 0;
      }  else {
 
  if(topsensor_status == 0 && bottomsensor_status == 1){
      relay_status = false;
      relay_state = LOW;
      } /*else {
  
  if(topsensor_status == 1 && bottomsensor_status == 0 && flag == 1){
      relay_status = false; */
      
      
      }
  }
  
  digitalWrite(relayPin, relay_state);
  digitalWrite(ledpin, relay_state);
}

void manualTrigger() {
  button_reading = digitalRead(manualOnPin);
  // if the input just went from LOW and HIGH and we've waited long enough
  // to ignore any noise on the circuit, toggle the output pin and remember
  // the time
  if (button_reading == LOW && previous == HIGH && millis() - timeButton > debounce) {
    if (relay_status == true) {
      relay_status = false;
      relay_state = HIGH; }
    else {
      relay_status = true;
      relay_state = LOW; }
      
    timeButton = millis();    
  }  
  digitalWrite(relayPin, relay_state);
  digitalWrite(ledpin, relay_state);
  previous = button_reading;
}  

void displayboot() {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15,0);
  display.print("SWAPNIL");
  //display.setTextSize(1);
  //display.setTextColor(WHITE);
  display.setCursor(0,25);
  display.print("WATERLEVEL");
  display.setCursor(0,50);
  display.print("CONTROLLER");
  display.display();
  delay(5000);
}
