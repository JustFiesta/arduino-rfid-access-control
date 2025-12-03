#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// OLED - dla 128x64
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Możliwe adresy: 0x3C (0x78>>1) lub 0x3D (0x7A>>1)
#define OLED_ADDRESS 0x3C

// RFID
#define RST_PIN 9
#define SS_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);

// SERVO
#define SERVO_PIN 6
Servo myservo;

// BUZZER
#define BUZZER_PIN 5

// AUTORYZOWANE KARTY - używamy char[] zamiast String (oszczędność RAM)
const char* authorizedCards[] = {
  "007:7B:2A:25",  // Karta biala
  "69:51:45:14",  // Karta niebieska
};

const char* cardNames[] = {
  "Admin",
  "User",
};

const int numAuthorizedCards = 2;

void setup() {
  Serial.begin(9600);
  
  Serial.println(F("\n=== RFID Access Control ==="));

  // --- RFID ---
  SPI.begin();
  mfrc522.PCD_Init();
  delay(30);

  Serial.println(F("\n--- RFID Status ---"));
  byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  if (version == 0x00 || version == 0xFF) {
    Serial.println(F("ERROR: RFID not connected!"));
  } else {
    Serial.print(F("RFID OK - Firmware: 0x"));
    Serial.println(version, HEX);
  }

  // --- I2C ---
  Serial.println(F("\n--- I2C Scan ---"));
  Wire.begin();

  int devices = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("I2C device at 0x"));
      if (addr < 16) Serial.print(F("0"));
      Serial.println(addr, HEX);
      devices++;
    }
  }

  if (devices == 0) {
    Serial.println(F("WARNING: No I2C devices found!"));
  }

  // --- OLED ---
  Serial.println(F("\n--- OLED Status ---"));
  
  // Próba inicjalizacji OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("OLED init failed at 0x3C, trying 0x3D..."));
    
    // Próba z alternatywnym adresem
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println(F("OLED FAILED!"));
      Serial.println(F("Possible causes:"));
      Serial.println(F("1. Wrong address (check jumper on OLED)"));
      Serial.println(F("2. Not enough RAM (need ~1KB free)"));
      Serial.println(F("3. Bad connections SDA/SCL"));
      
      // Kontynuuj bez OLED (opcjonalnie)
      // for(;;); // Odkomentuj aby zatrzymać program
    } else {
      Serial.println(F("OLED OK at 0x3D!"));
    }
  } else {
    Serial.println(F("OLED OK at 0x3C!"));
  }
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println(F("Access"));
  display.println(F("Control"));
  display.setTextSize(1);
  display.setCursor(0, 50);
  display.println(F("Ready..."));
  display.display();

  // --- SERVO ---
  myservo.attach(SERVO_PIN);
  myservo.write(0);
  Serial.println(F("\n--- Servo Status ---"));
  Serial.println(F("Servo: LOCKED (0°)"));

  // --- BUZZER ---
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println(F("\n--- Buzzer Status ---"));
  Serial.println(F("Buzzer: Ready"));
  
  // Krótki test buzzera
  tone(BUZZER_PIN, 1000, 100);
  delay(50);

  Serial.println(F("\n=== System Ready ==="));
  Serial.println(F("Waiting for RFID card...\n"));
  
  delay(100);
  
  // Ekran główny
  showMainScreen();
}

void showMainScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println(F("Scan your card"));
  display.setCursor(0, 30);
  display.println(F("to enter..."));
  display.display();
}

// === FUNKCJE BUZZERA ===

// Dźwięk powitalny (dostęp przyznany)
void playAccessGranted() {
  tone(BUZZER_PIN, 1000, 100);
  delay(120);
  tone(BUZZER_PIN, 1500, 100);
  delay(120);
  tone(BUZZER_PIN, 2000, 150);
  delay(200);
}

// Dźwięk otwarcia drzwi
void playDoorOpen() {
  tone(BUZZER_PIN, 800, 200);
  delay(250);
  tone(BUZZER_PIN, 1200, 200);
  delay(250);
}

// Dźwięk zamknięcia drzwi
void playDoorClose() {
  tone(BUZZER_PIN, 1200, 150);
  delay(200);
  tone(BUZZER_PIN, 800, 150);
  delay(200);
}

// Dźwięk błędu (dostęp odmówiony)
void playAccessDenied() {
  for(int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 200, 150);
    delay(200);
    noTone(BUZZER_PIN);
    // delay(100);
  }
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Odczytaj UID - używamy char array zamiast String
  char uidString[16];
  int pos = 0;
  
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (i > 0) uidString[pos++] = ':';
    
    byte value = mfrc522.uid.uidByte[i];
    if (value < 0x10) uidString[pos++] = '0';
    
    char hexChars[] = "0123456789ABCDEF";
    uidString[pos++] = hexChars[value >> 4];
    uidString[pos++] = hexChars[value & 0x0F];
  }
  uidString[pos] = '\0';

  Serial.println(F("=================="));
  Serial.print(F("Card detected! UID: "));
  Serial.println(uidString);

  // Sprawdź autoryzację
  bool authorized = false;
  int cardIndex = -1;
  
  for (int i = 0; i < numAuthorizedCards; i++) {
    if (strcmp(uidString, authorizedCards[i]) == 0) {
      authorized = true;
      cardIndex = i;
      break;
    }
  }

  if (authorized) {
    // DOSTĘP PRZYZNANY
    Serial.print(F("ACCESS GRANTED for: "));
    Serial.println(cardNames[cardIndex]);
    
    // Dźwięk powitania
    playAccessGranted();
    
    // OLED - Witaj
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.println(F("Welcome,"));
    display.setCursor(0, 35);
    display.println(cardNames[cardIndex]);
    display.display();
    
    delay(1000);
    
    // OLED - Otwieranie
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println(F("Opening door..."));
    display.display();
    
    // OTWÓRZ SERWO + Dźwięk
    Serial.println(F(">>> Opening lock (90°)"));
    playDoorOpen();
    myservo.write(90);
    delay(500);
    
    // ZAMKNIJ SERWO + Dźwięk
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println(F("Closing door..."));
    display.display();
    
    Serial.println(F(">>> Closing lock (0°)"));
    playDoorClose();
    myservo.write(0);
    
    delay(500);
    
  } else {
    // DOSTĘP ODMÓWIONY
    Serial.println(F("ACCESS DENIED!"));
    Serial.print(F("Unknown card: "));
    Serial.println(uidString);
    
    // Dźwięk błędu
    playAccessDenied();
    
    // OLED - Odmowa
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.println(F("ACCESS"));
    display.println(F("DENIED!"));
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.println(F("Unknown card"));
    display.display();
    
    delay(1500);
  }

  Serial.println(F("==================\n"));

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  showMainScreen();
  delay(500);
}