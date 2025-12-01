#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// RFID
#define RST_PIN 9
#define SS_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);

// SERVO
#define SERVO_PIN 6
Servo myservo;
unsigned long lastServoTime = 0;  // ostatni czas uruchomienia serwa
bool servoPos = false;             // pozycja serwa (false = 0°, true = 180°)

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("\n=== RFID + I2C + SERVO Test ===");

  // --- RFID ---
  SPI.begin();
  mfrc522.PCD_Init();
  delay(100);

  Serial.println("\n--- RFID Status ---");
  mfrc522.PCD_DumpVersionToSerial();

  byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  if (version == 0x00 || version == 0xFF) {
    Serial.println("ERROR: RFID not connected!");
  } else {
    Serial.print("RFID OK - Firmware: 0x");
    Serial.println(version, HEX);
  }

  // --- I2C ---
  Serial.println("\n--- I2C Scan ---");
  Wire.begin();

  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("I2C device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
    }
  }

  // --- LCD ---
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Hello World!");
  lcd.setCursor(0,1);
  lcd.print("I2C: 0x27");

  // --- SERVO ---
  myservo.attach(SERVO_PIN);
  myservo.write(90);     // pozycja startowa
  Serial.println("\n--- SERVO Status ---");
  Serial.println("Servo attached on D6");
  Serial.println("Set to 90 degrees");

  Serial.println("\n=== Ready ===");
  Serial.println("Scan your RFID card...\n");
}

void loop() {

  // --- SERWO co 3 sekundy ---
  unsigned long currentMillis = millis();
  if (currentMillis - lastServoTime >= 3000) {
    lastServoTime = currentMillis;
    if (servoPos) {
      myservo.write(0);  // ustaw serwo w 0°
      Serial.println("Servo -> 0 degrees");
    } else {
      myservo.write(180); // ustaw serwo w 180°
      Serial.println("Servo -> 180 degrees");
    }
    servoPos = !servoPos;
  }

  // RFID
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println("==================");
  Serial.print("Card detected! UID: ");

  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      Serial.print("0");
      uidString += "0";
    }
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    uidString += String(mfrc522.uid.uidByte[i], HEX);

    if (i < mfrc522.uid.size - 1) {
      Serial.print(":");
      uidString += ":";
    }
  }
  Serial.println();

  // Typ karty
  Serial.print("Card type: ");
  MFRC522::PICC_Type t = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(t));
  Serial.println("==================\n");

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  // LCD info
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Card detected!");
  lcd.setCursor(0,1);
  lcd.print(uidString);

  delay(1500);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Hello World!");
  lcd.setCursor(0,1);
  lcd.print("I2C: 0x27");
}
