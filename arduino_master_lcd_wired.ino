//************************************************************
// Water Level Controller V 1.0
//
// Master LCD Program
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

// Define Trig and Echo pin:
#define manualOnPin 2
#define relayPin 3
#define ledpin 4
#define TopSensor 9
#define BottomSensor 10
#define echoPin 11
#define trigPin 12

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Define variables:
long duration;
int distance;
int tankEmpty = 90;
bool relay_status = false;
int relay_state = HIGH;
String motor_status = "";
int button_reading;           // the current reading from the input pin
int previous = HIGH;    // the previous reading from the input pin
long timeButton = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers

const unsigned char MainLogo [] PROGMEM = {
  0xfe, 0x0f, 0xfc, 0xff, 0xfe, 0x0f, 0xf8, 0xff, 0xfe, 0x0f, 0xf8, 0x7f, 0xfe, 0x0f, 0xf0, 0x7f, 
  0xfe, 0x0f, 0xe0, 0x3f, 0xfe, 0x0f, 0xe0, 0x1f, 0xf0, 0x01, 0xc0, 0x1f, 0xf0, 0x03, 0x80, 0x0f, 
  0xf8, 0x03, 0xf0, 0x7f, 0xfc, 0x07, 0xf0, 0x7f, 0xfc, 0x0f, 0xf0, 0x7f, 0xfe, 0x0f, 0xf0, 0x7f, 
  0xff, 0x1f, 0xf0, 0x7f, 0xff, 0x3f, 0xf0, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xe3, 0xc7, 0xc7, 0xc0, 0x81, 0x81, 0x03, 
  0xc8, 0x00, 0x00, 0x13, 0xfe, 0x3c, 0x3c, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xe3, 0xc7, 0xc7, 
  0xc0, 0x81, 0x81, 0x03, 0xc8, 0x08, 0x10, 0x13, 0xfe, 0x3c, 0x3c, 0x7f, 0xff, 0xff, 0xff, 0xff, 
  0xe3, 0xe3, 0xc7, 0xc7, 0xc0, 0x81, 0x81, 0x03, 0xc8, 0x08, 0x10, 0x13, 0xfc, 0x3c, 0x3c, 0x3f
};

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

unsigned long previousTimeOled = millis();
long timeIntervalOled = 1;

unsigned long previousTimeMotor = millis();
long timeIntervalMotor = 1;

unsigned long previousTimeManual = millis();
long timeIntervalManual = 1;

// Setup
void setup() {
  // Begin Serial communication at a baudrate of 9600:
  Serial.begin(115200);
  
  // Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(TopSensor, INPUT);
  pinMode(BottomSensor, INPUT);
  pinMode(ledpin, OUTPUT);
  digitalWrite(ledpin, LOW);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);// initialize with the I2C addr 0x3D (for the 128x64)
  display.clearDisplay();

  displayBoot();
  displayProgressBar();
  // Clear the buffer.
  display.clearDisplay(); 
}

//Loop
void loop() {
  unsigned long currentTime = millis();
  // task 1
  if(currentTime - previousTimeOled > timeIntervalOled) {
    previousTimeOled = currentTime;
    //Read ultrasonic
    ultrasonicRead();
    display.clearDisplay();
    //Display Motor Status
    motorDisplayFunction();
    //Display Over Head Tank Level
    displayOhtLevel(SCREEN_HEIGHT, distance);
    display.display();
  }

  // task 2
  if(currentTime - previousTimeMotor > timeIntervalMotor) {
    previousTimeMotor = currentTime;
    //Set Motor status
    motorFunction();
  }
  
  // task 3
  if(currentTime - previousTimeManual > timeIntervalManual) {
    previousTimeManual = currentTime;
    //Manual Trigger
    manualTrigger();
  }
}

void displayBoot() {
  // Display bitmap
  display.drawBitmap(48, 0,  MainLogo, 32, 32, WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(35, 38);
  display.print("Water Level");
  display.setCursor(20, 50);
  display.print("Controller V1.0");
  display.display();
}

void displayProgressBar() {
  displayBoot();
  for (int p1 = 0; p1 <= 100; p1++) {
    drawProgressbar( 0, 58, 128, 6,p1); 
    delay(100);
    display.display();
  }
}

void drawProgressbar(int x,int y, int width,int height, int progress) {
   progress = progress > 100 ? 100 : progress;
   progress = progress < 0 ? 0 :progress;
   float bar = ((float)(width) / 100) * progress; 
   display.fillRect(x+2, y+2, bar , height, WHITE);
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
  int final_height = ((height*ohtLevel)/tankEmpty);
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

void motorFunction() {
  if(digitalRead(TopSensor) == HIGH && digitalRead(BottomSensor) == HIGH){
    //Both level low
    relay_status = false;
    relay_state = LOW;
  } else {
    if(digitalRead(TopSensor) == LOW && digitalRead(BottomSensor) == LOW){
      //Bottom level high & top level high
      relay_status = true;
      relay_state = HIGH;
    }
  }
  digitalWrite(relayPin, relay_state);
}

void manualTrigger() {
  button_reading = digitalRead(manualOnPin);
  // if the input just went from LOW and HIGH and we've waited long enough
  // to ignore any noise on the circuit, toggle the output pin and remember
  // the time
  if (button_reading == LOW && previous == HIGH && millis() - timeButton > debounce) {
    if (relay_status == true) {
      relay_status = false;
      relay_state = LOW; }
    else {
      relay_status = true;
      relay_state = HIGH; }
      
    timeButton = millis();    
  }  
  digitalWrite(relayPin, relay_state);
  previous = button_reading;
}

void motorDisplayFunction() {
  motor_status = "";
  if(relay_status) {
    motor_status = "ON";
  } else {
    motor_status = "OFF";
  }
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.drawBitmap(0, 0,  Motoricon, 32, 32, WHITE);
  display.setTextSize(2);
  display.setCursor(34,10);
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
}
