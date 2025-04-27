#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // Šírka OLED displeja v pixeloch
#define SCREEN_HEIGHT 64 // Výška OLED displeja v pixeloch
#define OLED_RESET    -1 // Resetný pin (nie je použitý, -1 ak nie je použitý)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int pin1 = A0; // Prvý pin pre meranie
const int pin2 = A1; // Druhý pin pre meranie
const int pin3 = A2; // Tretí pin pre meranie
const int buttonPin = 2; // Tlačidlo na spustenie detekcie

int lastButtonState = LOW; // Posledný stav tlačidla
int buttonState = LOW; // Aktuálny stav tlačidla
unsigned long lastDebounceTime = 0; // Čas poslednej debouncovej kontroly
unsigned long debounceDelay = 50; // Čas pre debounce

void setup() {
  Serial.begin(9600);

  // Inicializácia displeja
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000); // Pauza pre zobrazenie displeja
  display.clearDisplay();

  // Nastavíme tlačidlo ako vstup
  pinMode(buttonPin, INPUT);

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Press Start Button");
  display.display();
}

void loop() {
  // Sledovanie stavu tlačidla s debounce
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) { // Ak je tlačidlo stlačené, spustí sa detekcia
        startDetection();
      }
    }
  }

  lastButtonState = reading;
}

void startDetection() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Testing...");

  // Meranie diódy
  testDiode();

  // Meranie rezistora
  testResistor();

  // Meranie tranzistora
  testTransistor();

  // Meranie MOSFETu
  testMOSFET();

  display.display();
  delay(5000); // Zobraziť výsledok na 5 sekúnd
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Press Start Button");
  display.display();
}

void testDiode() {
  int p1 = analogRead(pin1);
  int p2 = analogRead(pin2);
  
  display.setCursor(0, 10);
  if (p1 > 500 && p2 < 500) {
    display.println("Diode: Anode->Pin1, Cathode->Pin2");
  } else if (p1 < 500 && p2 > 500) {
    display.println("Diode: Anode->Pin2, Cathode->Pin1");
  } else {
    display.println("No Diode detected");
  }
}

void testResistor() {
  int p1 = analogRead(pin1);
  int p2 = analogRead(pin2);

  // Výpočet odporu na základe napätia (kalkulácia v ohmoch)
  float resistance = (1023.0 / (float)(p1 - p2)) * 1000.0; // Jednoduchý výpočet
  display.setCursor(0, 20);
  display.print("Resistor: ");
  display.print(resistance);
  display.println(" Ohms");
}

void testTransistor() {
  int p1, p2, p3;
  
  // Nastav p1 ako bázu, skontroluj p2 a p3 ako kolektor/emitor
  pinMode(pin1, OUTPUT);
  digitalWrite(pin1, HIGH);
  delay(10);
  p2 = analogRead(pin2);
  p3 = analogRead(pin3);

  if (p2 > 500 && p3 < 500) {
    printResult("NPN", pin1, pin2, pin3);
  } else if (p2 < 500 && p3 > 500) {
    printResult("PNP", pin1, pin3, pin2);
  } else {
    pinMode(pin2, OUTPUT);
    digitalWrite(pin2, HIGH);
    delay(10);
    p1 = analogRead(pin1);
    p3 = analogRead(pin3);

    if (p1 > 500 && p3 < 500) {
      printResult("NPN", pin2, pin1, pin3);
    } else if (p1 < 500 && p3 > 500) {
      printResult("PNP", pin2, pin3, pin1);
    } else {
      display.setCursor(0, 30);
      display.println("No Transistor");
    }
  }

  pinMode(pin1, INPUT);
  pinMode(pin2, INPUT);
  pinMode(pin3, INPUT);
}

void testMOSFET() {
  int p1, p2, p3;

  pinMode(pin1, OUTPUT);
  pinMode(pin2, INPUT);
  pinMode(pin3, INPUT);

  digitalWrite(pin1, HIGH);
  delay(10);

  p1 = analogRead(pin1);
  p2 = analogRead(pin2);
  p3 = analogRead(pin3);

  if (p2 > 500) {
    display.setCursor(0, 40);
    display.println("MOSFET: Source -> Pin3, Drain -> Pin2");
  } else {
    display.setCursor(0, 40);
    display.println("No MOSFET detected");
  }
}

void printResult(String type, int base, int collector, int emitter) {
  display.setCursor(0, 50);
  display.print(type);
  display.println(" Transistor");

  display.setCursor(0, 60);
  display.print("Base: A");
  display.println(base - A0);

  display.setCursor(0, 70);
  display.print("Collector: A");
  display.println(collector - A0);

  display.setCursor(0, 80);
  display.print("Emitter: A");
  display.println(emitter - A0);
}
