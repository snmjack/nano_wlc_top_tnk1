//************************************************************
// Water Level Controller V 1.0
//
// Master LCD Program serial receiver
//************************************************************

// Necessary Libraries
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Necessary for OLED Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Define Relay Pin
# define relayPin 3
# define flowpin 4

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const size_t capacity = JSON_OBJECT_SIZE(3) + 75;
DynamicJsonDocument doc(capacity);

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

bool relay_status = false;
int relay_state = LOW;
char* motor_status;
char* flow_status;

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
  
  // Begin Serial communication at a baudrate of 9600:
  Serial.begin(4800);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);// initialize with the I2C addr 0x3D (for the 128x64)
  display.display();
  delay(5000);
  display.clearDisplay(); 
 }


 void loop() {
  
   unsigned long currentTime = millis();

   if (Serial.available() > 0) {
             //getJsonData();
          
   // Task 1 - Get Jason Data
      if(currentTime - previousTimeRec > timeIntervalRec) {
         previousTimeRec = currentTime;
          if (Serial.available() > 0) {
             getJsonData();
          }
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
  }
    else {
             display.setTextColor(WHITE);
             display.setTextSize(2);
             display.setCursor(0, 30);
             display.print("No Data Received");
             display.display();
          }
  }


  void getJsonData() {
  deserializeJson(doc, Serial);
  topsensor_status = doc["topsensor_status"];
  bottomsensor_status = doc["bottomsensor_status"];
  overheadlevel = doc["overheadlevel"];
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
  
}

void motorFunction() {
  
  display.setCursor(34,10);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  
  if(topsensor_status == 1 && bottomsensor_status == 1){
    relay_status = false;
    relay_state = LOW;
  } 
  
  if(topsensor_status == 0 && bottomsensor_status == 0){
      relay_status = true;
      relay_state = HIGH;
      }
  digitalWrite(relayPin, relay_state);
}

/*void flowfunction() {
  
  display.setCursor(0,40);
  display.setTextSize(2);
  display.setTextColor(WHITE);

  if(relay_state  == HIGH && flowpin == HIGH) {
    motor_status = "ON";
  } 
  if(relay_status  == false) {
    motor_status = "OFF";
  }
  if(topsensor_status == 1 && bottomsensor_status == 1){
    relay_status = false;
    relay_state = LOW;
  } 
  
  if(topsensor_status == 0 && bottomsensor_status == 0){
      relay_status = true;
      relay_state = HIGH;
      }
  digitalWrite(relayPin, relay_state);
}
*/
