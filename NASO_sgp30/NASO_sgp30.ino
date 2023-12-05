#include "SparkFun_SGP30_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET     4    // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
SGP30 mySensor; //create an object of the SGP30 class
 
void setup() 
{
  Serial.begin(9600);
  Wire.begin();
 
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  
  //Initialize sensor
  if (mySensor.begin() == false) 
  {
    
    display.setCursor(0, 0); //oled display
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.print("error");
    display.display();
//    display.clearDisplay();
    Serial.println("No SGP30 Detected. Check connections.");
    
    while (1);
  }
  //Initializes sensor for air quality readings
  //measureAirQuality should be called in one second increments after a call to initAirQuality
  mySensor.initAirQuality();
}
 
void loop() 
{
  //First fifteen readings will be
  //CO2: 400 ppm  TVOC: 0 ppb
  delay(1000); //Wait 1 second
  //measure CO2 and TVOC levels
  mySensor.measureAirQuality();
  Serial.print("CO2: ");
  Serial.print(mySensor.CO2);
  Serial.print(" ppm\tTVOC: ");
  Serial.print(mySensor.TVOC);
  Serial.println(" ppb");
 
  display.setCursor(0, 10); //oled display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print("CO2:");
  display.print(mySensor.CO2);
  display.setTextSize(1);
  display.print("ppm");
 
  display.setCursor(0, 40); //oled display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print("TVOC:");
  display.print(mySensor.TVOC);
  display.setTextSize(1);
  display.print("ppb");
 
  display.display();
  display.clearDisplay();
 
} 
