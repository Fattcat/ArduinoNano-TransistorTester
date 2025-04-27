#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Testovacie piny
const int PINS[3] = { A0, A1, A2 };

// Na meranie odporu cez delič (R1 medzi 5V a pinom, Rx medzi pinom a GND)
const float VIN       = 5.0;
const float KNOWN_R   = 10000.0;  // 10 kΩ

void setup() {
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 init failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Smart Tester Ready");
  display.display();
  delay(1000);
}

void loop() {
  display.clearDisplay();
  if      (detectDiode())      { /* vypísané vnútri */ }
  else if (detectResistor())   { /* vypísané vnútri */ }
  else if (detectTransistor()) { /* vypísané vnútri */ }
  else if (detectMOSFET())     { /* vypísané vnútri */ }
  else {
    display.setCursor(0,20);
    display.println("No Component");
    display.display();
  }
  delay(5000);
}

//------------------ DETEKCIA DIÓDY ------------------
bool detectDiode() {
  // skúšame každú permutáciu: jedna noha OUTPUT=HIGH, druhá meraná
  for (int i=0; i<3; i++) {
    int anodePin = PINS[i];
    int cathodePin = PINS[(i+1)%3];
    int unusedPin = PINS[(i+2)%3];

    // nastavíme meraný pin pull-down (INPUT)
    pinMode(cathodePin, INPUT);
    // anódu na HIGH
    pinMode(anodePin, OUTPUT);
    digitalWrite(anodePin, HIGH);
    delay(10);
    int vCath = analogRead(cathodePin);
    pinMode(anodePin, INPUT);

    if (vCath > 700) {
      // prúd tečie z anódy do katódy
      display.setCursor(0,10);
      display.println("Diode Detected");
      display.setCursor(0,30);
      display.print("A:"); display.println(anodePin - A0);
      display.setCursor(0,40);
      display.print("K:"); display.println(cathodePin - A0);
      display.display();
      return true;
    }
  }
  return false;
}

//------------------ DETEKCIA REZISTORA ------------------
bool detectResistor() {
  for (int i=0; i<3; i++) {
    int pin = PINS[i];
    // predpoklad: medzi 5V a pinom KNOWN_R, Rx medzi pinom a GND
    pinMode(pin, INPUT);
    int raw = analogRead(pin);
    float Vout = raw * (VIN / 1023.0);
    if (Vout < 0.01 || Vout > VIN-0.01) continue;
    float Rx = (Vout * KNOWN_R) / (VIN - Vout);
    if (Rx > 1.0 && Rx < 1e6) {
      display.setCursor(0,10);
      display.println("Resistor Detected");
      display.setCursor(0,30);
      display.print("Pins: A"); display.print(pin - A0);
      display.print(" - ?");  // nevieš ktorej druhej nohe, pretože rezistor len medzi GND a pinom
      display.setCursor(0,40);
      display.print("Value: "); display.print(Rx,1); display.println(" Ohm");
      display.display();
      return true;
    }
  }
  return false;
}

//------------------ DETEKCIA TRANZISTORA ------------------
bool detectTransistor() {
  int vcol, vem;
  for (int b=0; b<3; b++) {
    int base = PINS[b];
    int col  = PINS[(b+1)%3];
    int em   = PINS[(b+2)%3];
    pinMode(base, OUTPUT);
    digitalWrite(base, HIGH);
    delay(10);
    vcol = analogRead(col);
    vem  = analogRead(em);
    pinMode(base, INPUT);
    if (vcol > 700 && vem < 300) {
      printTransistor("NPN", base,col,em);
      return true;
    }
    if (vcol < 300 && vem > 700) {
      printTransistor("PNP", base,em,col);
      return true;
    }
  }
  return false;
}

void printTransistor(const char* type, int b, int c, int e) {
  display.setCursor(0,10);
  display.print(type); display.println(" Transistor");
  display.setCursor(0,30);
  display.print("B:"); display.println(b - A0);
  display.setCursor(0,40);
  display.print("C:"); display.println(c - A0);
  display.setCursor(0,50);
  display.print("E:"); display.println(e - A0);
  display.display();
}

//------------------ DETEKCIA MOSFETU ------------------
bool detectMOSFET() {
  int v1, v2;
  // skúška pre N-kanál: Gate=HIGH, meraj Drain vs Source
  for (int i=0; i<3; i++) {
    int gate = PINS[i];
    int drain = PINS[(i+1)%3];
    int src   = PINS[(i+2)%3];
    // nastav Gate
    pinMode(gate, OUTPUT);
    digitalWrite(gate, HIGH);
    delay(10);
    // meraj na drain/source
    pinMode(drain, INPUT);
    pinMode(src, INPUT);
    v1 = analogRead(drain);
    v2 = analogRead(src);
    pinMode(gate, INPUT);
    if (v1 > 700 && v2 < 300) {
      printMOSFET("N-Channel", gate, drain, src);
      return true;
    }
    // skúška pre P-kanál: Gate=LOW (teda pull-down)
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
  display.setCursor(0,10);
  display.print(type); display.println(" MOSFET");
  display.setCursor(0,30);
  display.print("G:"); display.println(g - A0);
  display.setCursor(0,40);
  display.print("D:"); display.println(d - A0);
  display.setCursor(0,50);
  display.print("S:"); display.println(s - A0);
  display.display();
}
