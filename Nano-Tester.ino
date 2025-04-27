#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Testovací tlačidlo
const int START_BUTTON_PIN = 2;  // digitálny pin s tlačidlom (pripojený k GND pri stlačení)

// Testovacie piny pre komponenty
const int PINS[3] = { A0, A1, A2 };

// Parametre pre meranie odporu v deličovom zapojení
const float VIN       = 5.0;
const float KNOWN_R   = 10000.0;  // 10 kΩ

void setup() {
  Serial.begin(115200);

  // Inicializácia OLED displeja
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 init failed"));
    for (;;);
  }

  // Nastavenie tlačidla s interným pull-up odporom
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Smart Tester Ready");
  display.setCursor(0, 20);
  display.println("Press Start Button");
  display.display();
}

void loop() {
  // Čakáme na stlačenie tlačidla
  if ( waitForStart() ) {
    display.clearDisplay();
    // Spustíme testovanie komponentu
    if      (detectDiode())      { /* výpis interný */ }
    else if (detectResistor())   { /* výpis interný */ }
    else if (detectTransistor()) { /* výpis interný */ }
    else if (detectMOSFET())     { /* výpis interný */ }
    else {
      display.setCursor(0,20);
      display.println("No Component");
      display.display();
    }
    delay(5000);
    // Po teste vrátime prompt
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Smart Tester Ready");
    display.setCursor(0,20);
    display.println("Press Start Button");
    display.display();
  }
}

// Čaká na stlačenie tlačidla s debouncingom
bool waitForStart() {
  while (true) {
    // tlačidlo stlačené = LOW
    if (digitalRead(START_BUTTON_PIN) == LOW) {
      delay(50); // debouncing
      if (digitalRead(START_BUTTON_PIN) == LOW) {
        // počkáme na uvoľnenie
        while (digitalRead(START_BUTTON_PIN) == LOW) ;
        delay(50);
        return true;
      }
    }
  }
}

// --------------------------------------------------
// DETEKCIA DIÓDY
// --------------------------------------------------
bool detectDiode() {
  for (int i = 0; i < 3; i++) {
    int anodePin   = PINS[i];
    int cathodePin = PINS[(i + 1) % 3];

    pinMode(anodePin, OUTPUT);
    digitalWrite(anodePin, HIGH);
    delay(10);
    pinMode(cathodePin, INPUT);
    int vCath = analogRead(cathodePin);
    pinMode(anodePin, INPUT);

    if (vCath > 700) {
      display.clearDisplay();
      display.setCursor(0,10);
      display.println("Diode Detected");
      display.setCursor(0,30);
      display.print("A: A"); display.println(anodePin - A0);
      display.setCursor(0,40);
      display.print("K: A"); display.println(cathodePin - A0);
      display.display();
      return true;
    }
  }
  return false;
}

// --------------------------------------------------
// DETEKCIA REZISTORA
// --------------------------------------------------
bool detectResistor() {
  for (int i = 0; i < 3; i++) {
    int pin = PINS[i];
    pinMode(pin, INPUT);
    int raw = analogRead(pin);
    float Vout = raw * (VIN / 1023.0);
    if (Vout < 0.01 || Vout > VIN - 0.01) continue;
    float Rx = (Vout * KNOWN_R) / (VIN - Vout);
    if (Rx > 1.0 && Rx < 1e6) {
      display.clearDisplay();
      display.setCursor(0,10);
      display.println("Resistor Detected");
      display.setCursor(0,30);
      display.print("Pin: A"); display.println(pin - A0);
      display.setCursor(0,40);
      display.print("Value: "); display.print(Rx,1); display.println(" Ohm");
      display.display();
      return true;
    }
  }
  return false;
}

// --------------------------------------------------
// DETEKCIA TRANZISTORA
// --------------------------------------------------
bool detectTransistor() {
  int vcol, vem;
  for (int b = 0; b < 3; b++) {
    int base = PINS[b];
    int col  = PINS[(b + 1) % 3];
    int em   = PINS[(b + 2) % 3];

    pinMode(base, OUTPUT);
    digitalWrite(base, HIGH);
    delay(10);
    vcol = analogRead(col);
    vem  = analogRead(em);
    pinMode(base, INPUT);

    if (vcol > 700 && vem < 300) {
      printTransistor("NPN", base, col, em);
      return true;
    }
    if (vcol < 300 && vem > 700) {
      printTransistor("PNP", base, em, col);
      return true;
    }
  }
  return false;
}

void printTransistor(const char* type, int b, int c, int e) {
  display.clearDisplay();
  display.setCursor(0,10);
  display.print(type); display.println(" Transistor");
  display.setCursor(0,30);
  display.print("B: A"); display.println(b - A0);
  display.setCursor(0,40);
  display.print("C: A"); display.println(c - A0);
  display.setCursor(0,50);
  display.print("E: A"); display.println(e - A0);
  display.display();
}

// --------------------------------------------------
// DETEKCIA MOSFETU
// --------------------------------------------------
bool detectMOSFET() {
  int v1, v2;
  for (int i = 0; i < 3; i++) {
    int gate  = PINS[i];
    int drain = PINS[(i + 1) % 3];
    int src   = PINS[(i + 2) % 3];

    // Test N-kanál: Gate HIGH
    pinMode(gate, OUTPUT);
    digitalWrite(gate, HIGH);
    delay(10);
    pinMode(drain, INPUT);
    pinMode(src, INPUT);
    v1 = analogRead(drain);
    v2 = analogRead(src);
    pinMode(gate, INPUT);
    if (v1 > 700 && v2 < 300) {
      printMOSFET("N-Channel", gate, drain, src);
      return true;
    }

    // Test P-kanál: Gate LOW
    pinMode(gate, OUTPUT);
    digitalWrite(gate, LOW);
    delay(10);
    v1 = analogRead(src);
    v2 = analogRead(drain);
    pinMode(gate, INPUT);
    if (v1 > 700 && v2 < 300) {
      printMOSFET("P-Channel", gate, src, drain);
      return true;
    }
  }
  return false;
}

void printMOSFET(const char* type, int g, int d, int s) {
  display.clearDisplay();
  display.setCursor(0,10);
  display.print(type); display.println(" MOSFET");
  display.setCursor(0,30);
  display.print("G: A"); display.println(g - A0);
  display.setCursor(0,40);
  display.print("D: A"); display.println(d - A0);
  display.setCursor(0,50);
  display.print("S: A"); display.println(s - A0);
  display.display();
}
