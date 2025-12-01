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

// AUTORYZOWANE KARTY - dodaj swoje UID tutaj
String authorizedCards[] = {
  "07:7B:2A:25",  // Karta biala (inna dla kazdego)
  "69:51:45:14",  // Karta niebieska (krazek)
};

String cardNames[] = {
  "Admin",        // Nazwa dla karty 1
  "User",         // Nazwa dla karty 2
};

const int numAuthorizedCards = 3;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("\n=== RFID Access Control ===");

  // --- RFID ---
  SPI.begin();
  mfrc522.PCD_Init();
  delay(100);

  Serial.println("\n--- RFID Status ---");
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

  int devices = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("I2C device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      devices++;
    }
  }

  if (devices == 0) {
    Serial.println("WARNING: No I2C devices found!");
  }

  // --- LCD ---
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Control");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");

  // --- SERVO ---
  myservo.attach(SERVO_PIN);
  myservo.write(0);  // Pozycja ZAMKNIĘTE
  Serial.println("\n--- Servo Status ---");
  Serial.println("Servo: LOCKED (0°)");

  Serial.println("\n=== System Ready ===");
  Serial.println("Waiting for RFID card...\n");
  
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan your card");
  lcd.setCursor(0, 1);
  lcd.print("to enter...");
}

void loop() {
  // Sprawdź czy karta jest obecna
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Odczytaj UID karty
  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      uidString += "0";
    }
    uidString += String(mfrc522.uid.uidByte[i], HEX);
    
    if (i < mfrc522.uid.size - 1) {
      uidString += ":";
    }
  }
  uidString.toUpperCase();  // Konwersja na wielkie litery

  Serial.println("==================");
  Serial.print("Card detected! UID: ");
  Serial.println(uidString);

  // Sprawdź czy karta jest autoryzowana
  bool authorized = false;
  int cardIndex = -1;
  
  for (int i = 0; i < numAuthorizedCards; i++) {
    if (uidString == authorizedCards[i]) {
      authorized = true;
      cardIndex = i;
      break;
    }
  }

  if (authorized) {
    // DOSTĘP PRZYZNANY
    Serial.print("ACCESS GRANTED for: ");
    Serial.println(cardNames[cardIndex]);
    
    // LCD - Witaj
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Welcome,");
    lcd.setCursor(0, 1);
    lcd.print(cardNames[cardIndex] + "!");
    
    delay(1500);
    
    // LCD - Otwieranie
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Opening door...");
    
    // OTWÓRZ SERWO
    Serial.println(">>> Opening lock (90°)");
    myservo.write(90);
    delay(3000);  // Drzwi otwarte przez 3 sekundy
    
    // ZAMKNIJ SERWO
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Closing door...");
    Serial.println(">>> Closing lock (0°)");
    myservo.write(0);
    
    delay(1000);
    
  } else {
    // DOSTĘP ODMÓWIONY
    Serial.println("ACCESS DENIED!");
    Serial.print("Unknown card: ");
    Serial.println(uidString);
    
    // LCD - Odmowa
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ACCESS DENIED!");
    lcd.setCursor(0, 1);
    lcd.print("Unknown card");
    
    delay(2000);
  }

  Serial.println("==================\n");

  // Zatrzymaj kartę
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  // Powrót do ekranu głównego
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan your card");
  lcd.setCursor(0, 1);
  lcd.print("to enter...");

  delay(1000);  // Krótkie opóźnienie przed następnym skanem
}