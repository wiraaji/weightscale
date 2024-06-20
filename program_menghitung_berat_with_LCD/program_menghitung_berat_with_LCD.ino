#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <HX711.h>

MCUFRIEND_kbv tft;
HX711 scale;

#define DOUT 10
#define CLK 11
#define BTN_PIN A5  // Pin untuk tombol print

// Definisikan pin untuk LED dan buzzer
#define LAMPU_MERAH 12
#define LAMPU_HIJAU 13

int NYALA = HIGH; 
int MATI = LOW;

float calibration_factor = 386.53;  // Faktor kalibrasi tetap yang telah diketahui

void setup() {
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("Initializing...");

  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor);
  scale.tare();

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 10);
  tft.setTextSize(3);
  tft.print("Taring scale...");
  delay(2000);

  tft.fillScreen(TFT_BLACK);

  // Set pin untuk LED sebagai OUTPUT
  pinMode(LAMPU_MERAH, OUTPUT);
  pinMode(LAMPU_HIJAU, OUTPUT);

  // Set pin untuk tombol tare sebagai INPUT_PULLUP
  pinMode(BTN_PIN, INPUT_PULLUP);

  // Hapus pesan "Taring scale..."
  tft.fillRect(10, 10, 300, 30, TFT_BLACK);  // Bersihkan area pesan
}

void loop() {
  // Cek apakah tombol tare ditekan (LOW aktif karena INPUT_PULLUP)
  if (digitalRead(BTN_PIN) == LOW) {
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

  float currentWeight = scale.get_units(10);

  // Pembulatan nilai berat tanpa angka di belakang koma
  int roundedWeight = round(currentWeight);

  // Tampilkan nilai berat pada layar TFT
  tft.fillRect(10, 40, 300, 30, TFT_BLACK);  // Bersihkan area untuk nilai berat
  tft.setCursor(10, 40);
  tft.print("Weight : ");
  tft.print(roundedWeight);  // Tampilkan nilai yang telah dibulatkan
  tft.print(" g");

  // Kontrol LED sesuai dengan kondisi berat
  if (roundedWeight <= 1) {
    digitalWrite(LAMPU_HIJAU, MATI);
    digitalWrite(LAMPU_MERAH, MATI);
  } else if (roundedWeight > 1 && roundedWeight <= 180) {
    digitalWrite(LAMPU_HIJAU, MATI);
    digitalWrite(LAMPU_MERAH, NYALA);
  } else if (roundedWeight > 180 && roundedWeight <= 230) {
    digitalWrite(LAMPU_HIJAU, NYALA);
    digitalWrite(LAMPU_MERAH, MATI);
    // digitalWrite(BUZZER_PIN, MATI);
  } else {
    digitalWrite(LAMPU_HIJAU, LOW);
    digitalWrite(LAMPU_MERAH, NYALA);
  }

  delay(100);  // Delay untuk mengurangi flicker
}
