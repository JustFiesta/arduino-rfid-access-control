#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// RFID
#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("\n=== RFID + I2C Test ===");

  // RFID init
  SPI.begin();
  mfrc522.PCD_Init();
  delay(100);

  Serial.println("\n--- RFID Status ---");
  mfrc522.PCD_DumpVersionToSerial();

  byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  if (version == 0x00 || version == 0xFF) {
    Serial.println("ERROR: RFID not connected!");
    Serial.println("Check wiring!");
  } else {
    Serial.print("RFID OK - Firmware: 0x");
    Serial.println(version, HEX);
  }

  // I2C test
  Serial.println("\n--- I2C Scan ---");
  Wire.begin();
  Wire.setClock(100000);

  int devices = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      devices++;
    }
  }
  if (devices == 0) {
    Serial.println("No I2C devices found - LCD problem!");
  }

  // LCD init
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Hello World!");
  lcd.setCursor(0,1);
  lcd.print("I2C: 0x27");

  Serial.println("\n=== Ready ===");
  Serial.println("Scan your RFID card...\n");
}

void loop() {
  // Sprawdź czy karta jest obecna
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  
  // Odczytaj kartę
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Wypisz UID karty
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
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  Serial.println("==================\n");

  // Stop komunikacji z kartą
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
