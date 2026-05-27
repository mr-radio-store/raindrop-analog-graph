/*
Raindrop module analog output for graph

Wiring connection
1. Rain Sensor Module	Arduino	Notes
VCC	5V	Power supply
GND	GND	Ground
DO (digital out)	D2 (example)	HIGH when dry, LOW when wet
AO (analog out)	A0 (optional)	Returns analog moisture level

2. OLED wire connecction
Wiring: I2C OLED
OLED Pin	Arduino Uno / Mega
VCC	5V
GND	GND
SDA	A4 (Uno) / 20 (Mega)
SCL	A5 (Uno) / 21 (Mega)

*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define RAIN_SENSOR_AO_PIN A0
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int MAX_ANALOG = 700;

// Time-series buffer
const uint8_t TS_WIDTH = 100;
uint8_t graphBuffer[TS_WIDTH] = {0};
uint8_t graphIndex = 0;

// Gauge parameters (top-left)
const int gaugeCenterX = 32;
const int gaugeCenterY = 24;
const int gaugeRadius = 20;

// Thermometer parameters (top-right)
const int thermoX = 70;
const int thermoY = 10;
const int thermoWidth = 50;
const int thermoHeight = 10;

void setup() {
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED not found"));
    while (true);
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  int rainValue = analogRead(RAIN_SENSOR_AO_PIN);
  int clamped = constrain(rainValue, 0, MAX_ANALOG);

  // Scale for thermometer and time-series
  int thermoFill = map(clamped, 0, MAX_ANALOG, 0, thermoWidth);
  int tsValue = map(clamped, 0, MAX_ANALOG, 0, 30);
  graphBuffer[graphIndex] = tsValue;

  display.clearDisplay();

  // === 1. Gauge (Analog Meter) ===
  float angle = map(clamped, 0, MAX_ANALOG, 135, 45);
  float rad = angle * PI / 180.0;
  int needleX = gaugeCenterX + gaugeRadius * cos(rad);
  int needleY = gaugeCenterY - gaugeRadius * sin(rad);

  // Gauge Arc
  for (int a = 45; a <= 135; a += 5) {
    float arc = a * PI / 180.0;
    int x = gaugeCenterX + gaugeRadius * cos(arc);
    int y = gaugeCenterY - gaugeRadius * sin(arc);
    display.drawPixel(x, y, SSD1306_WHITE);
  }

  // Gauge Ticks
  for (int a = 45; a <= 135; a += 15) {
    float arc = a * PI / 180.0;
    int x1 = gaugeCenterX + (gaugeRadius - 2) * cos(arc);
    int y1 = gaugeCenterY - (gaugeRadius - 2) * sin(arc);
    int x2 = gaugeCenterX + (gaugeRadius + 2) * cos(arc);
    int y2 = gaugeCenterY - (gaugeRadius + 2) * sin(arc);
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }

  // Gauge Needle
  display.drawLine(gaugeCenterX, gaugeCenterY, needleX, needleY, SSD1306_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Gauge");

  // === 2. Thermometer Bar (Horizontal) ===
  display.drawRect(thermoX, thermoY, thermoWidth, thermoHeight, SSD1306_WHITE);
  display.fillRect(thermoX + 1, thermoY + 1, thermoFill, thermoHeight - 2, SSD1306_WHITE);
  display.setCursor(thermoX, thermoY - 8);
  display.setTextSize(1);
  display.print("Thermo");

  // Value label below Thermometer
  display.setTextSize(1);
  display.setCursor(thermoX, thermoY + thermoHeight + 2);
  display.print("Val:");
  display.setTextSize(2);
  display.setCursor(thermoX + 24, thermoY + thermoHeight + 1);
  display.print(clamped);

  // === 3. Time-Series Graph (Bottom) ===
  const int tsX = 14;
  const int tsY = SCREEN_HEIGHT - 1;
  const int tsHeight = 30;

  display.drawRect(tsX, tsY - tsHeight, TS_WIDTH, tsHeight, SSD1306_WHITE);
  for (uint8_t i = 1; i < TS_WIDTH; i++) {
    int idx1 = (graphIndex + i - 1) % TS_WIDTH;
    int idx2 = (graphIndex + i) % TS_WIDTH;
    int y1 = tsY - graphBuffer[idx1];
    int y2 = tsY - graphBuffer[idx2];
    display.drawLine(tsX + i - 1, y1, tsX + i, y2, SSD1306_WHITE);
  }

  display.display();
  graphIndex = (graphIndex + 1) % TS_WIDTH;
  delay(200);
}
