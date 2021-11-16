//Include modules
#include <Wire.h>
#include <Adafruit_AM2315.h>
#include <FastLED.h>

//***ARDUINO WIRE CONNECTIONS***
// Connect RED of the AM2315 sensor to 5.0V
// Connect BLACK to Ground
// Connect WHITE to i2c clock (A5)
// Connect YELLOW to i2c data (A4)
// Connect LED data to pin 7 (D7)
// Connect Water sensor to pin 2 (D2)

// intiialize temp & hum sensor
Adafruit_AM2315 am2315; 

//sensor pin for water level
int water_level_pin = 2;

//set up LED controller
#define LEDPIN 7
#define NUMOFLEDS 24
CRGB leds[NUMOFLEDS];
String led_mode = "off";

void setup() {
  FastLED.addLeds<WS2812B, LEDPIN, GRB>(leds, NUMOFLEDS);
  Serial.begin(9600);
  while (!Serial) {
    delay(10);
  }
  
  if (! am2315.begin()) {
     Serial.println("Sensor not found, check wiring & pullups!");
     while (1);
  }

  pinMode(water_level_pin, INPUT_PULLUP);

  //off (none, looping)
  if (led_mode == "off"){
    for (int i = 0; i <= 23; i++) {
      leds[i] = CRGB (0, 0, 0);
      FastLED.show();
      delay(40);
    }
  }
}

int waterSig4 = 0;
int waterSig3 = 0;
int waterSig2 = 0;
int waterSig1 = 0;
int waterSig0 = 0;
 
void loop() {
  //Serial Data Out
  float temperature, humidity;
  int water_low;

  if (! am2315.readTemperatureAndHumidity(&temperature, &humidity)) {
    temperature = -1;
    humidity = -1;
    return;
  }

  if (digitalRead(water_level_pin) == HIGH)
  {
    waterSig0 = waterSig1;
    waterSig1 = waterSig2;
    waterSig2 = waterSig3;
    waterSig3 = waterSig4;
    waterSig4 = 1;
  }
  else
  {
    waterSig0 = waterSig1;
    waterSig1 = waterSig2;
    waterSig2 = waterSig3;
    waterSig3 = waterSig4;
    waterSig4 = 0;
  }

  if (waterSig4 == 1 || waterSig3 == 1 || waterSig2 == 1 || waterSig1 == 1 || waterSig0 == 1)
  {
    water_low = 1;
  }
  else
  {
    water_low = 0;
  }
  
  Serial.print(humidity);
  Serial.print(" "); 
  Serial.print(temperature*(1.800)+32); //need to manually adjust the sensor smh
  Serial.print(" ");
  Serial.print(water_low);
  Serial.println(); 

  //Mode Management
  if (Serial.available() > 0) {
    String led_data = Serial.readStringUntil('\n');
    led_mode = led_data;
  }
  
  //Connected, Running (green, looping)
  if (led_mode =="connected_running"){
    for (int i = 0; i <= 23; i++) {
      leds[i] = CRGB (0, 10, 0);
      FastLED.show();
      delay(40);
    }
    for (int i = 0; i <= 23; i++) {
      leds[i] = CRGB (0, 5, 0);
      FastLED.show();
      delay(40);
    } 
  }

  //Connected, Idle (green, static)
  if (led_mode =="connected_idle"){
    for (int i = 0; i <= 23; i++) {
      leds[i] = CRGB (0, 5, 0);
      FastLED.show();
      delay(40);
    }  
  }

  //Island, Running (white, looping)
  if (led_mode =="offline_running"){
    for (int i = 0; i <= 23; i++) {
      leds[i] = CRGB (10, 10, 10);
      FastLED.show();
      delay(40);
    }
    for (int i = 0; i <= 23; i++) {
      leds[i] = CRGB (5, 5, 5);
      FastLED.show();
      delay(40);
    } 
  }

  //Island, Idle (white, static)
  if (led_mode =="offline_idle"){
    for (int i = 0; i <= 23; i++) {
      leds[i] = CRGB (5, 5, 5);
      FastLED.show();
      delay(40);
    }  
  }

  //Master sends error message (red, static)
  if (led_mode =="error"){
    for (int i = 0; i <= 23; i++) {
      leds[i] = CRGB (5, 0, 0);
      FastLED.show();
      delay(40);
    }  
  }

  //AP-mode, Server accepting connections (blue, looping)
  if (led_mode =="accepting_wifi_connection"){
    for (int i = 0; i <= 23; i++) {
      leds[i] = CRGB (0, 0, 10);
      FastLED.show();
      delay(40);
    }
    for (int i = 0; i <= 23; i++) {
      leds[i] = CRGB (0, 0, 5);
      FastLED.show();
      delay(40);
    } 
  }

  //off (none, looping)
  if (led_mode == "off"){
    for (int i = 0; i <= 23; i++) {
      leds[i] = CRGB (0, 0, 0);
      FastLED.show();
      delay(40);
    }
  }

  Serial.println(led_mode);  
  
  delay(1);
}
