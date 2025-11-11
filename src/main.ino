// Reapply 3
// RFID (simulated by button) + PIN demo 
// Keypad 4x4: rows -> A0,A1,A2,A3 ; cols -> 2,3,4,5
// I2C LCD: A4(SDA), A5(SCL) (address 0x27 typical)
// Green LED -> D6, Buzzer -> D7, Relay_sim (LED) -> D8
// Simulated RFID button -> D9 (INPUT_PULLUP; press to GND)

#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// pins
const int LED_OK = 6;
const int BUZZER = 7;
const int RELAY_SIM = 8;
const int CARD_BUTTON = 9; // simulated RFID

// keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {A0, A1, A2, A3}; // R1..R4
byte colPins[COLS] = {2, 3, 4, 5};     // C1..C4
Keypad keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Simple DB: UIDs (4 bytes) mapped to PIN strings (we simulate UID on button press)
struct User {
  byte uid[4];
  const char* pin;
  const char* name;
};

User users[] = {
  {{0xDE,0xAD,0xBE,0xEF}, "1234", "Alice"},
  {{0xAA,0xBB,0xCC,0xDD}, "0000", "Bob"}
};
const int USER_COUNT = sizeof(users)/sizeof(users[0]);

// security
int failedAttempts = 0;
const int MAX_FAILED = 3;
unsigned long lockoutUntil = 0;
const unsigned long LOCKOUT_MS = 60UL * 1000UL; // 60 seconds lockout

String enteredPIN = "";
const unsigned long OPEN_DURATION_MS = 3000; // simulate open 3s

void setup() {
  Serial.begin(115200);
  pinMode(LED_OK, OUTPUT); digitalWrite(LED_OK, LOW);
  pinMode(BUZZER, OUTPUT); digitalWrite(BUZZER, LOW);
  pinMode(RELAY_SIM, OUTPUT); digitalWrite(RELAY_SIM, LOW);
  pinMode(CARD_BUTTON, INPUT_PULLUP); // button to GND

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("RFID+PIN DEMO");
  lcd.setCursor(0,1);
  lcd.print("Ready");
  delay(800);
}

void loop() {
  if (millis() < lockoutUntil) {
    lcd.clear();
    lcd.print("LOCKED: wait");
    lcd.setCursor(0,1);
    int s = (int)((lockoutUntil - millis())/1000);
    lcd.print(s);
    delay(200);
    return;
  }

  // simulate card detection via button
  if (digitalRead(CARD_BUTTON) == LOW) { // pressed -> card detected
    // choose which UID to simulate: long press cycles? for now use first UID
    byte readUid[4] = {0xDE, 0xAD, 0xBE, 0xEF}; // simulated UID
    Serial.print("Simulated Card UID: ");
    for (byte i=0;i<4;i++){ Serial.print(readUid[i], HEX); Serial.print(" "); }
    Serial.println();

    int idx = findUserByUid(readUid);
    if (idx < 0) {
      deny("Card not allowed");
      delay(300); // simple debounce / prevention of bouncing
      while (digitalRead(CARD_BUTTON)==LOW) delay(10); // wait release
      return;
    }

    // card allowed -> ask for PIN
    lcd.clear();
    lcd.print("Card: ");
    lcd.print(users[idx].name);
    lcd.setCursor(0,1);
    lcd.print("Enter PIN: ");
    enteredPIN = "";
    bool pinOk = getPinFromKeypadWithTimeout(15000, enteredPIN); // 15s to enter
    if (!pinOk) {
      deny("PIN timeout");
      while (digitalRead(CARD_BUTTON)==LOW) delay(10);
      return;
    }
    if (enteredPIN.equals(String(users[idx].pin))) {
      grant(users[idx].name);
      failedAttempts = 0;
    } else {
      failedAttempts++;
      if (failedAttempts >= MAX_FAILED) {
        lockoutUntil = millis() + LOCKOUT_MS;
        lcd.clear(); lcd.print("Too many tries!");
      }
      deny("Bad PIN");
    }

    // wait for button release to avoid multiple readings
    while (digitalRead(CARD_BUTTON)==LOW) delay(10);
  }

  delay(50);
}

int findUserByUid(byte *u) {
  for (int i=0;i<USER_COUNT;i++) {
    bool ok = true;
    for (int j=0;j<4;j++) if (users[i].uid[j] != u[j]) ok = false;
    if (ok) return i;
  }
  return -1;
}

bool getPinFromKeypadWithTimeout(unsigned long timeoutMs, String &outPin) {
  unsigned long start = millis();
  outPin = "";
  lcd.setCursor(0,1);
  lcd.print("PIN: ");
  lcd.setCursor(5,1);
  while (millis() - start < timeoutMs) {
    char k = keypad.getKey();
    if (k) {
      if (k == '#') { // confirm
        // short beep
        tone(BUZZER, 1200, 80);
        return outPin.length() > 0;
      } else if (k == '*') { // clear
        outPin = "";
        lcd.setCursor(5,1);
        lcd.print("    ");
        lcd.setCursor(5,1);
        tone(BUZZER, 800, 60);
      } else if (isDigit(k) && outPin.length() < 8) {
        outPin += k;
        // show asterisks on LCD
        lcd.setCursor(5 + outPin.length() - 1,1);
        lcd.print('*');
        tone(BUZZER, 1000, 60);
      }
    }
    delay(20);
  }
  return false; // timeout
}

void grant(const char* who) {
  Serial.println("Access granted for " + String(who));
  lcd.clear();
  lcd.print("Welcome ");
  lcd.print(who);
  lcd.setCursor(0,1);
  lcd.print("Access granted");
  digitalWrite(LED_OK, HIGH);
  tone(BUZZER, 2000, 150);
  digitalWrite(RELAY_SIM, HIGH); // simulate unlock
  delay(OPEN_DURATION_MS);
  digitalWrite(RELAY_SIM, LOW);
  digitalWrite(LED_OK, LOW);
  lcd.clear();
  lcd.print("Ready");
}

void deny(const char* msg) {
  Serial.println(String("Access denied: ") + msg);
  lcd.clear();
  lcd.print("Access denied");
  lcd.setCursor(0,1);
  lcd.print(msg);
  // beep/blink
  for (int i=0;i<2;i++) {
    tone(BUZZER, 800, 200);
    delay(250);
  }
  delay(600);
  lcd.clear();
  lcd.print("Ready");
}
