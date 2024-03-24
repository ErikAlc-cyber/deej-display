#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int NUM_SLIDERS = 4;
const int NUM_OF_LAYERS = 2;
const int analogInputs[NUM_SLIDERS] = {A0, A1, A2, A3};

//make a buffer to avoid jitter
const int BUFFER_SIZE = 5;
int analogBuffer[NUM_SLIDERS][BUFFER_SIZE];

int displayVolume[NUM_SLIDERS];
int analogSliderValues[NUM_SLIDERS];
String analogSliderNames[NUM_SLIDERS] = {"Master","Mic","Music","Other"};

const unsigned long sleepAfter = 15000; // this value will change how long the oled will display until turning off.
unsigned long startTime;
unsigned long currTime;

//see http://javl.github.io/image2cpp/ for how to make these
const unsigned char sys [] PROGMEM = {
  0xff, 0x3f, 0xc0, 0xff, 0x3f, 0xc0, 0xff, 0x3f, 0xc0, 0xff, 0x3f, 0xc0, 0xff, 0x3f, 0xc0, 0xff,
  0x3f, 0xc0, 0xff, 0x3f, 0xc0, 0xff, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x3f,
  0xc0, 0xff, 0x3f, 0xc0, 0xff, 0x3f, 0xc0, 0xff, 0x3f, 0xc0, 0xff, 0x3f, 0xc0, 0xff, 0x3f, 0xc0,
  0xff, 0x3f, 0xc0, 0xff, 0x3f, 0xc0
};

const unsigned char music [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x07, 0xff, 0x00, 0x07, 0xff, 0x00, 0x07,
  0xff, 0x00, 0x07, 0xe3, 0x00, 0x06, 0x03, 0x00, 0x04, 0x03, 0x00, 0x04, 0x03, 0x00, 0x04, 0x03,
  0x00, 0x04, 0x1f, 0x00, 0x0c, 0x3f, 0x00, 0x7c, 0x3e, 0x00, 0x7c, 0x3c, 0x00, 0x7c, 0x00, 0x00,
  0x78, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char microphone [] PROGMEM = {
	0x00, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x03, 0xf0, 0x00, 0x03, 0xf0, 0x00, 0x03, 0xf0, 0x00, 0x03, 
	0xf0, 0x00, 0x03, 0xf0, 0x00, 0x03, 0xf0, 0x00, 0x0b, 0xf4, 0x00, 0x0b, 0xf4, 0x00, 0x0f, 0xfc, 
	0x00, 0x04, 0xc8, 0x00, 0x03, 0xf0, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xc0, 0x00, 
	0x01, 0xe0, 0x00, 0x00, 0x00, 0x00
};

const unsigned char all [] PROGMEM = {
	0x00, 0x60, 0x00, 0x01, 0xe0, 0x00, 0x03, 0xe0, 0x00, 0x07, 0xe3, 0x00, 0x3f, 0xe1, 0x80, 0x7f, 
	0xec, 0x80, 0xff, 0xec, 0xc0, 0xff, 0xe6, 0xc0, 0xff, 0xe6, 0xc0, 0xff, 0xe6, 0xc0, 0xff, 0xe6, 
	0xc0, 0xff, 0xec, 0xc0, 0x7f, 0xec, 0x80, 0x3f, 0xe1, 0x80, 0x07, 0xe3, 0x00, 0x03, 0xe0, 0x00, 
	0x01, 0xe0, 0x00, 0x00, 0x60, 0x00
};

const unsigned char *const icons[] PROGMEM = {sys, microphone, music, all}; // array with all icons

void setup() { 
  for (int i = 0; i < NUM_SLIDERS; i++) {
    pinMode(analogInputs[i], INPUT);
  }

  Serial.begin(9600);
  analogReadResolution(12);

  for (int i = 0; i < NUM_SLIDERS; i++) {
    for (int j = 0; j < BUFFER_SIZE; j++) {
      analogBuffer[i][j] = analogRead(analogInputs[i]);
    }
  }

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000); 
  startTime = millis();
}

void loop() {
  updateSliderValues();
  sendSliderValues(); // Actually send data (all the time)
  // printSliderValues(); // For debug
  if (currTime - startTime >= sleepAfter) {
    display.clearDisplay();
    display.display();
    startTime = currTime;
  } 
  delay(10);
}

void updateSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    // Read analog value of slider
    int rawValue = analogRead(analogInputs[i]);

    // Update the buffer value
    for (int j = 0; j < BUFFER_SIZE - 1; j++) {
      analogBuffer[i][j] = analogBuffer[i][j + 1];
    }
    analogBuffer[i][BUFFER_SIZE - 1] = rawValue;

    // Calculate average buffer 
    int avgValue = 0;
    for (int j = 0; j < BUFFER_SIZE; j++) {
      avgValue += analogBuffer[i][j];
    }
    avgValue /= BUFFER_SIZE;

    // Update Slider value if change is great
    if (abs(avgValue - displayVolume[i]) > 30) {
      displayVolume[i] = avgValue;
      displayVol(i);
      display.display();
      startTime = currTime;
    }
  }
}

void sendSliderValues() {
  String builtString = String(""); 
  
  
  for (int i = 0; i < NUM_SLIDERS; i++) {
    builtString += String((int)analogSliderValues[i]);
    if (i < NUM_SLIDERS - 1) {
      builtString += String("|");
    }
  }
  Serial.println(builtString);
}

void displayVol(int i){
  display.clearDisplay();
  
  int percentage = percentage_volume(displayVolume[i]);
  display.drawBitmap(10, 2, icons[i], 18, 18, WHITE);
  display.fillRect(10, 25, percentage/1.5, 10, WHITE);
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(35, 5);             // Start at top-left corner
  display.println(analogSliderNames[i]);
  display.setCursor(90,25);
  display.setTextSize(2);
  display.println(percentage);
  display.display();
}

int percentage_volume(int actual_value){
  return (actual_value * 100) / 4095;
}

void printSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    String printedString = String("Slider #") + String(i + 1) + String(": ") + String(analogSliderValues[i]) + String(" mV");
    Serial.write(printedString.c_str());

    if (i < NUM_SLIDERS - 1) {
      Serial.write(" | ");
    } else {
      Serial.write("\n");
    }
  }
}