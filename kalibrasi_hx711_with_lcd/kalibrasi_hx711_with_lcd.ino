#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <HX711.h>

MCUFRIEND_kbv tft;
HX711 scale;

#define DOUT 10
#define CLK 11
#define CALIBRATION_WEIGHT 200.0  // Berat dalam gram yang digunakan untuk kalibrasi

float calibration_factor = 1.0;  // Faktor kalibrasi awal


void setup() {
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("Calibrating...");

  scale.begin(DOUT, CLK);
  scale.tare();

  tft.setCursor(10, 40);
  tft.print("Place ");
  tft.print(CALIBRATION_WEIGHT);
  tft.print("g weight");
  delay(5000);

  long reading = scale.get_value(10);
  calibration_factor = reading / CALIBRATION_WEIGHT;
  scale.set_scale(calibration_factor);

  tft.fillScreen(TFT_RED);
  tft.setCursor(10, 10);
  tft.setTextSize(2);
  tft.print("Calibration done");

  tft.setCursor(10, 40);
  tft.print("Cal. Factor: ");
  tft.print(calibration_factor);
  delay(5000);
}
void loop() {
}
