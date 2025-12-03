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

// Emotki dla każdego użytkownika (weeb style)
const char* cardEmojis[] = {
  "(=^w^=)",      // Admin - kot
  "(^ω^)",   // User - szczęśliwy
};

const int numAuthorizedCards = 2;

void setup() {
  Serial.begin(9600);
  
  Serial.println(F("\n=== RFID Access Control ==="));

  // --- RFID ---
  SPI.begin();
  mfrc522.PCD_Init();
  delay(100);

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
  delay(150);

  Serial.println(F("\n=== System Ready ==="));
  Serial.println(F("Waiting for RFID card...\n"));
  
  delay(2000);
  
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

// Dźwięk powitalny (dostęp przyznany) - anime "kyun~" sound
void playAccessGranted() {
  // Cute sparkle notification
  tone(BUZZER_PIN, 1319, 60);  // E6
  delay(70);
  tone(BUZZER_PIN, 1568, 60);  // G6
  delay(70);
  tone(BUZZER_PIN, 1976, 60);  // B6
  delay(70);
  tone(BUZZER_PIN, 2637, 100); // E7 - high sparkle
  delay(120);
  tone(BUZZER_PIN, 2093, 80);  // C7
  delay(100);
  tone(BUZZER_PIN, 2637, 120); // E7 again
  delay(150);
}

// Dźwięk otwarcia drzwi - victory fanfare (FF style)
void playDoorOpen() {
  // Epic win melody
  tone(BUZZER_PIN, 1047, 120); // C6
  delay(140);
  tone(BUZZER_PIN, 1047, 120); // C6
  delay(140);
  tone(BUZZER_PIN, 1047, 120); // C6
  delay(140);
  tone(BUZZER_PIN, 1047, 200); // C6 sustained
  delay(220);
  tone(BUZZER_PIN, 1319, 120); // E6
  delay(140);
  tone(BUZZER_PIN, 1568, 300); // G6 - triumphant!
  delay(350);
}

// Dźwięk zamknięcia drzwi - "mata ne~" goodbye
void playDoorClose() {
  // Gentle descending lullaby
  tone(BUZZER_PIN, 1568, 150); // G6
  delay(170);
  tone(BUZZER_PIN, 1319, 150); // E6
  delay(170);
  tone(BUZZER_PIN, 1175, 150); // D6
  delay(170);
  tone(BUZZER_PIN, 1047, 250); // C6 - soft ending
  delay(280);
  tone(BUZZER_PIN, 988, 150);  // B5
  delay(170);
}

// Dźwięk błędu (dostęp odmówiony) - "dame dame" (nope nope)
void playAccessDenied() {
  // Sad descending "wrong answer" buzzer
  tone(BUZZER_PIN, 587, 200);  // D5
  delay(220);
  tone(BUZZER_PIN, 494, 200);  // B4
  delay(220);
  tone(BUZZER_PIN, 440, 200);  // A4
  delay(220);
  tone(BUZZER_PIN, 330, 400);  // E4 - sad low note
  delay(450);
  noTone(BUZZER_PIN);
  delay(150);
  // Dramatic pause then...
  tone(BUZZER_PIN, 294, 150);  // D4 - even sadder
  delay(180);
  tone(BUZZER_PIN, 262, 300);  // C4 - defeated
  delay(350);
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
    
    // OLED - Animacja powitania (weeb style)
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(20, 2);
    display.println(F("~ Welcome ~"));
    
    display.setTextSize(2);
    display.setCursor(10, 20);
    display.print(cardNames[cardIndex]);
    display.println(F("-san"));
    
    display.setTextSize(1);
    display.setCursor(25, 45);
    display.println(cardEmojis[cardIndex]);
    display.display();
    
    delay(1000);
    
    // OLED - Otwieranie (fancy)
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(15, 10);
    display.println(F(">> Unlocking <<"));
    display.setCursor(30, 28);
    display.println(F("Door..."));
    display.setCursor(25, 45);
    display.println(F("( -_-)>"));
    display.display();
    
    // OTWÓRZ SERWO + Dźwięk
    Serial.println(F(">>> Opening lock (90°)"));
    playDoorOpen();
    myservo.write(90);
    delay(500);
    
    // ZAMKNIJ SERWO + Dźwięk
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(15, 10);
    display.println(F(">> Locking <<"));
    display.setCursor(25, 28);
    display.println(F("Stay safe!"));
    display.setCursor(30, 45);
    display.println(F("(^_^)b"));
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
    display.setCursor(25, 48);
    display.println(F("(T_T)"));
    display.display();
    
    delay(1500);
  }

  Serial.println(F("==================\n"));

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  showMainScreen();
  delay(500);
}