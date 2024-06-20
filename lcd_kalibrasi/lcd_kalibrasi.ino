#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <HX711.h>
#include <Adafruit_Thermal.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

#define TX_PIN 1  // Connect to printer RX
#define RX_PIN 0  // Connect to printer TX

MCUFRIEND_kbv tft;
HX711 scale;
SoftwareSerial mySerial(RX_PIN, TX_PIN);  // RX, TX
Adafruit_Thermal printer(&mySerial);

#define DOUT 10
#define CLK 11
#define CALIBRATION_WEIGHT 200.0  // Berat dalam gram yang digunakan untuk kalibrasi
#define BTN_PIN A5                // Pin untuk tombol

#define LAMPU_MERAH 12
#define LAMPU_HIJAU 13

int NYALA = HIGH;
int MATI = LOW;

float calibration_factor = 58.71;  // Faktor kalibrasi awal
const int EEPROM_ADDRESS = 0;  // Alamat EEPROM untuk menyimpan calibration_factor

void setup() {
  Serial.begin(9600);  // Inisialisasi serial untuk debug
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(-1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("Initializing...");

  scale.begin(DOUT, CLK);

  // Baca nilai calibration_factor dari EEPROM
  EEPROM.get(EEPROM_ADDRESS, calibration_factor);

  scale.set_scale(calibration_factor);
  scale.tare();

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 10);
  tft.setTextSize(3);
  tft.print("Taring scale...");
  delay(2000);

  tft.fillScreen(TFT_BLACK);

  pinMode(LAMPU_MERAH, OUTPUT);
  pinMode(LAMPU_HIJAU, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  mySerial.begin(19200);
  printer.begin();

  tft.fillRect(10, 10, 300, 30, TFT_BLACK);  // Bersihkan area pesan
}

void calibrateScale() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 10);
  tft.setTextSize(2);
  tft.print("Calibrating...");

  scale.tare();

  tft.setCursor(10, 40);
  tft.print("Place ");
  tft.print(CALIBRATION_WEIGHT);
  tft.print("g weight");
  delay(10000);

  long reading = scale.get_value(10);
  calibration_factor = reading / CALIBRATION_WEIGHT;
  scale.set_scale(calibration_factor);

  // Simpan nilai calibration_factor ke EEPROM
  EEPROM.put(EEPROM_ADDRESS, calibration_factor);

  tft.fillScreen(TFT_RED);
  tft.setCursor(10, 10);
  tft.setTextSize(2);
  tft.print("Calibration done");

  tft.setCursor(10, 40);
  tft.print("Cal. Factor: ");
  tft.print(calibration_factor);
  delay(5000);

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 10);
  tft.setTextSize(3);
  tft.print("Taring scale...");
  scale.tare();
  delay(2000);

  tft.fillScreen(TFT_BLACK);
}

void tareScale() {
  scale.tare();
  tft.fillRect(10, 10, 300, 30, TFT_BLACK);  // Bersihkan area pesan
  tft.setCursor(10, 10);
  tft.print("Taring...");

  delay(1000);  // Tunggu 1 detik untuk tare selesai

  tft.fillRect(10, 10, 300, 30, TFT_BLACK);  // Bersihkan area pesan
  tft.setCursor(10, 10);
  tft.print("Tare done");

  delay(2000);  // Tampilkan pesan "Tare done" selama 2 detik

  tft.fillRect(10, 10, 300, 30, TFT_BLACK);  // Bersihkan area pesan lagi
}

void printWeight() {
  tft.fillRect(10, 10, 300, 30, TFT_BLACK);  // Bersihkan area pesan
  tft.setCursor(10, 10);
  tft.print("Printing");

  float currentWeight = getStableWeight();

  printer.setSize('L');
  printer.setFont('A');
  printer.println("Weight Measurement");
  printer.print("Weight: ");
  printer.print(currentWeight);
  printer.println(" g");
  printer.println();
  printer.setSize('M');
  printer.println("IT NSI © 2024");
  printer.feed(2);

  tft.fillRect(10, 10, 300, 30, TFT_BLACK);
  tft.setCursor(10, 10);
  tft.print("Printing done");

  delay(1000);

  tft.fillRect(10, 10, 300, 30, TFT_BLACK);
}

float getStableWeight() {
  const int numReadings = 10;
  float readings[numReadings];
  float total = 0;

  for (int i = 0; i < numReadings; i++) {
    readings[i] = scale.get_units();
    total += readings[i];
    delay(50);  // Delay kecil antara pembacaan untuk stabilisasi
  }

  float average = total / numReadings;
  return average;
}

void loop() {
  static unsigned long buttonPressTime = 0;
  static bool buttonPressed = false;
  static bool isTaring = false;
  static bool isCalibrating = false;

  if (digitalRead(BTN_PIN) == LOW) {
    if (!buttonPressed) {
      buttonPressed = true;
      buttonPressTime = millis();
      Serial.println("Button pressed");
    }

    unsigned long currentTime = millis();
    unsigned long pressedDuration = currentTime - buttonPressTime;

    if (pressedDuration > 4000 && !isCalibrating) {
      Serial.println("Starting calibration...");
      calibrateScale();
      isCalibrating = true;
      isTaring = false;
      buttonPressed = false;
    } else if (pressedDuration > 2500 && !isTaring && !isCalibrating) {
      Serial.println("Starting tare...");
      tareScale();
      isTaring = true;
      buttonPressed = false;
    }
  } else {
    if (buttonPressed) {
      unsigned long pressedDuration = millis() - buttonPressTime;
      
        Serial.println("Printing weight...");
        printWeight();
      
      buttonPressed = false;
      isTaring = false;
      isCalibrating = false;
    }
  }

  float currentWeight = getStableWeight();

  tft.fillRect(10, 40, 300, 30, TFT_BLACK);  // Bersihkan area untuk nilai berat
  tft.setCursor(10, 40);
  tft.print("Weight : ");
  tft.print(currentWeight);  // Tampilkan nilai yang telah dibulatkan
  tft.print(" g");

  if (currentWeight <= 1) {
    digitalWrite(LAMPU_HIJAU, MATI);
    digitalWrite(LAMPU_MERAH, MATI);
  } else if (currentWeight > 1 && currentWeight <= 180) {
    digitalWrite(LAMPU_HIJAU, MATI);
    digitalWrite(LAMPU_MERAH, NYALA);
  } else if (currentWeight > 180 && currentWeight <= 230) {
    digitalWrite(LAMPU_HIJAU, NYALA);
    digitalWrite(LAMPU_MERAH, MATI);
  } else {
    digitalWrite(LAMPU_HIJAU, MATI);
    digitalWrite(LAMPU_MERAH, NYALA);
  }

  delay(100);
}