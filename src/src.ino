#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  Serial.println("Start LCD test...");
  
  // Test połączenia I2C
  Wire.begin();
  Wire.beginTransmission(0x27);
  byte error = Wire.endTransmission();
  Serial.print("I2C connection test: ");
  Serial.println(error == 0 ? "OK" : "FAILED");
  
  // Inicjalizacja LCD
  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("Hello World!");
  
  lcd.setCursor(0, 1);
  lcd.print("I2C: 0x27");
  
  Serial.println("LCD initialized");
}

void loop() {}
