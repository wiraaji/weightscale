#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <HX711.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

#define TX_PIN 13  // Connect to printer RX
#define RX_PIN 12  // Connect to printer TX

#define DOUT 10
#define CLK 11
#define CALIBRATION_WEIGHT 200.0  // Berat dalam gram yang digunakan untuk kalibrasi
#define BUZZER 1
#define BTN_PIN A5

MCUFRIEND_kbv tft;
HX711 scale;
SoftwareSerial PRINTER(RX_PIN, TX_PIN);  // RX, TX

int NYALA = HIGH;
int MATI = LOW;

float calibration_factor = 58.67;  // Faktor kalibrasi awal
const int EEPROM_ADDRESS = 0;      // Alamat EEPROM untuk menyimpan calibration_factor

unsigned long buttonPressStartTime = 0;
bool buttonPressed = false;

void setup() {
  // Serial.begin(9600);  // Inisialisasi serial untuk debug
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(-1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
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
  delay(1000);

  tft.fillScreen(TFT_BLACK);

  pinMode(BUZZER, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  PRINTER.begin(9600);
  tft.fillRect(10, 10, 300, 30, TFT_BLACK);  // Bersihkan area pesan
}

void calibrateScale() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(105, 10);
  tft.setTextSize(2);
  tft.print("Mengkalibrasi");
  delay(500);
  tft.setCursor(80, 30);
  tft.print("Kosongkan Timbangan");
  delay(2000);
  scale.tare();

  tft.fillScreen(TFT_BLACK);

  tft.setCursor(45, 40);
  tft.print("Tempatkan Benda ");
  tft.print(CALIBRATION_WEIGHT);
  tft.print(" gram");
  delay(2000);
  long reading = scale.get_value(10);
  calibration_factor = reading / CALIBRATION_WEIGHT;
  scale.set_scale(calibration_factor);
  // Simpan nilai calibration_factor ke EEPROM
  EEPROM.put(EEPROM_ADDRESS, calibration_factor);
  delay(7000);

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(80, 30);
  tft.print("Kosongkan Timbangan");
  delay(2000);
  scale.tare();
  delay(2000);
  tft.fillScreen(TFT_GREEN);
  tft.setCursor(80, 30);
  tft.setTextColor(TFT_BLACK);
  tft.print("Kalibrasi Berhasil");
  delay(1000);
}

void tareScale() {
  tft.setCursor(120, 170);
  tft.setTextSize(2);
  tft.print("Tare Scaling");
  scale.tare();
  delay(1000);  
  tft.setCursor(165, 190);
  tft.print("Done");
  delay(500);
}

void printWeight() {

  tft.setCursor(140, 170);
  tft.setTextSize(2);
  tft.print("Printing");

  float stableWeight = getStableWeight();

  char weightStr[10];
  dtostrf(stableWeight / 1000.0, 6, 3, weightStr);  // Convert to kilograms for display

  // Inisialisasi koneksi SoftwareSerial untuk printer
  SoftwareSerial mySerial(RX_PIN, TX_PIN);
  mySerial.begin(9600);

  mySerial.print("^XA");       // Start of label format
  mySerial.print("^PW380");    // Set label width in dots (2.8 cm * 203 dpi = 560 dots)
  mySerial.print("^LL800");    // Set label length in dots (5.3 cm * 203 dpi = 800 dots)
  mySerial.print("^FO20,20");  // Set origin of text

  // Set font size for the first line
  mySerial.print("^A0N,27,27");  // Set font: Font A, size 30

  // Print the first line
  mySerial.print("^FDPT NIHON SEIKI INDONESIA^FS");  // Print text

  // Second line (adjusted position and font)
  mySerial.print("^FO20,100");    // Set origin of second line
  mySerial.print("^A0N,27,27");   // Set font for second line (Font A, size 30)
  mySerial.print("^FDWeight: ");  // Print second line text
  mySerial.print(weightStr);      // Print the current weight
  mySerial.print("Kg^FS");

  mySerial.print("^XZ");  // End of label format
  delay(1500);
  tft.setCursor(165, 190);
  tft.print("Done");
  delay(500);
}

float roundToNearest(float value, float resolution) {
  return round(value / resolution) * resolution;
}

float getStableWeight() {
  const int numReadings = 1;
  float readings[numReadings];
  float total = 0;

  for (int i = 0; i < numReadings; i++) {
    readings[i] = scale.get_units(10);
    total += readings[i];
  }

  float average = total / numReadings;

  // Membulatkan nilai average sesuai dengan digit terakhirnya
  average = roundToNearest(average, 0.1);

  // Memastikan bahwa nilai stableWeight tidak kurang dari 0.09
  float stableWeight = (average < 0.09) ? 0 : average;

  return stableWeight;
}

void loop() {
  if (digitalRead(BTN_PIN) == LOW) {
    if (!buttonPressed) {
      buttonPressed = true;
      buttonPressStartTime = millis();
    }
  } else {
    if (buttonPressed) {
      buttonPressed = false;
      unsigned long pressDuration = millis() - buttonPressStartTime;

      if (pressDuration >= 5000) {
        calibrateScale();
      } else if (pressDuration >= 2000) {
        tareScale();
      } else if (pressDuration < 2000) {
        printWeight();
      }
    }
  }

  float stableWeight = getStableWeight();

  char weightStr[10];
  dtostrf(stableWeight / 1000.0, 6, 3, weightStr); 

  if (stableWeight <= 1) {
    digitalWrite(BUZZER, MATI);
    tft.fillScreen(TFT_BLACK);    
    tft.setTextColor(TFT_WHITE);  
  } else if (stableWeight < 6899) {
    digitalWrite(BUZZER, NYALA);
    delay(500);
    digitalWrite(BUZZER, MATI);
    tft.fillScreen(TFT_RED);      
    tft.setTextColor(TFT_WHITE);  
  } else if (stableWeight >= 6900 && stableWeight <= 7150) {
    digitalWrite(BUZZER, MATI);
    tft.fillScreen(TFT_GREEN);   
    tft.setTextColor(TFT_BLACK);  
  } else if (stableWeight > 7151) {
    digitalWrite(BUZZER, NYALA);
    delay(500);
    digitalWrite(BUZZER, MATI);
    tft.fillScreen(TFT_RED);      
    tft.setTextColor(TFT_WHITE); 
  }

  tft.fillRect(30, 100, 0, 0, TFT_BLACK);

  tft.setCursor(30, 100);
  tft.setTextSize(6);
  tft.print(weightStr);
  tft.print(" Kg");
}
