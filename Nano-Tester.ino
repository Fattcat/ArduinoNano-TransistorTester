#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Analógové piny, na ktorých testujeme súčiastky
const int pins[3] = { A0, A1, A2 };

// Pre meranie odporu cez delič: R1 medzi 5 V a Analógovým pinom,
// Rx (testovaný rezistor) medzi pinom a GND
const float VIN          = 5.0;      // referenčné napájacie napätie
const float KNOWN_R      = 10000.0;  // R1 = 10 kΩ

void setup() {
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Smart Component Tester");
  display.display();
  delay(1000);
}

void loop() {
  display.clearDisplay();

  // 1) Skúsime detekciu tranzistora
  if ( testTransistor() ) {
    // ak našiel, už zobrazil výsledok a končíme
    delay(5000);
    return;
  }

  // 2) Ak to nie je tranzistor, skúsime zmerať rezistor
  if ( testResistor() ) {
    delay(5000);
    return;
  }

  // 3) Ak ani rezistor, vypíšeme "No Component"
  display.setCursor(0, 10);
  display.println("No Component Detected");
  display.display();
  delay(5000);
}

// --------------------------------------------
// Funkcia testTransistor: vráti true, ak niečo deteguje
// --------------------------------------------
bool testTransistor() {
  int v[3];

  // Pre každú permutáciu pinov: jeden pin OUTPUT=HIGH, dva INPUT
  for (int b = 0; b < 3; b++) {
    int basePin = pins[b];
    int colPin  = pins[(b + 1) % 3];
    int emPin   = pins[(b + 2) % 3];

    pinMode(basePin, OUTPUT);
    digitalWrite(basePin, HIGH);
    delay(10);

    v[(b + 1) % 3] = analogRead(colPin);
    v[(b + 2) % 3] = analogRead(emPin);

    pinMode(basePin, INPUT);

    // Prahové hodnoty pre ADC (0–1023): 
    // >700 = vysoké napätie, <300 = nízke
    if (v[(b + 1) % 3] > 700 && v[(b + 2) % 3] < 300) {
      printTransistor("NPN", basePin, colPin, emPin);
      return true;
    }
    if (v[(b + 1) % 3] < 300 && v[(b + 2) % 3] > 700) {
      printTransistor("PNP", basePin, emPin, colPin);
      return true;
    }
  }
  return false;
}

// --------------------------------------------------
// Funkcia testResistor: vráti true, ak našla rezistor
// --------------------------------------------------
bool testResistor() {
  for (int i = 0; i < 3; i++) {
    float R = measureResistance(pins[i]);
    // ak je odpor v rozumnom rozsahu 10 Ω – 1 MΩ
    if (R > 10.0 && R < 1e6) {
      display.setCursor(0, 10);
      display.print("Resistor: ");
      display.print(R, 1);
      display.println(" Ohm");
      display.display();
      return true;
    }
  }
  return false;
}

// --------------------------------------------------
// Meranie odporu cez delič (R1 = KNOWN_R, Rx = meraný)
// --------------------------------------------------
float measureResistance(int pin) {
  int raw = analogRead(pin);
  float Vout = raw * (VIN / 1023.0);
  // ak Vout príliš blízko 0V alebo VIN, vynecháme
  if (Vout < 0.01 || Vout > VIN - 0.01) return -1.0;
  // Rx = (Vout * R1) / (Vin - Vout)
  return (Vout * KNOWN_R) / (VIN - Vout);
}

// --------------------------------------------------
// Pomocná funkcia: zobrazí typ tranzistora a nožičky
// --------------------------------------------------
void printTransistor(const char* type, int basePin, int colPin, int emPin) {
  display.setCursor(0, 10);
  display.print(type);
  display.println(" Transistor");

  display.setCursor(0, 30);
  display.print("Base: A");
  display.println(basePin - A0);

  display.setCursor(0, 40);
  display.print("Col: A");
  display.println(colPin - A0);

  display.setCursor(0, 50);
  display.print("Em: A");
  display.println(emPin - A0);

  display.display();
}
