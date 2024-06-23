// here I will give my code, sooo dont worry ... :D
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // Šírka OLED displeja v pixeloch
#define SCREEN_HEIGHT 64 // Výška OLED displeja v pixeloch
#define OLED_RESET    -1 // Resetný pin (nie je použitý, -1 ak nie je použitý)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int pin1 = A0;
const int pin2 = A1;
const int pin3 = A2;

void setup() {
  Serial.begin(9600);

  // Inicializácia displeja
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000); // Pauza pre zobrazenie displeja
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Transistor Tester");
  display.display();
}

void loop() {
  display.clearDisplay();
  testTransistor();
  delay(5000); // Čaká 5 sekúnd pred ďalším testom
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
    // Nastav p2 ako bázu, skontroluj p1 a p3 ako kolektor/emitor
    pinMode(pin1, INPUT);
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
      // Nastav p3 ako bázu, skontroluj p1 a p2 ako kolektor/emitor
      pinMode(pin2, INPUT);
      pinMode(pin3, OUTPUT);
      digitalWrite(pin3, HIGH);
      delay(10);
      p1 = analogRead(pin1);
      p2 = analogRead(pin2);

      if (p1 > 500 && p2 < 500) {
        printResult("NPN", pin3, pin1, pin2);
      } else if (p1 < 500 && p2 > 500) {
        printResult("PNP", pin3, pin2, pin1);
      } else {
        display.setCursor(0, 10);
        display.println("No Transistor");
      }
    }
  }

  pinMode(pin1, INPUT);
  pinMode(pin2, INPUT);
  pinMode(pin3, INPUT);
  display.display();
}

void printResult(String type, int base, int collector, int emitter) {
  display.setCursor(0, 10);
  display.print(type);
  display.println(" Transistor");

  display.setCursor(0, 30);
  display.print("Base: A");
  display.println(base - A0);

  display.setCursor(0, 40);
  display.print("Collector: A");
  display.println(collector - A0);

  display.setCursor(0, 50);
  display.print("Emitter: A");
  display.println(emitter - A0);
}
