/*
  Program Timbangan
*/

#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <HX711.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>

#define TX_PIN 13  // Connect to printer RX
#define RX_PIN 12  // Connect to printer TX

#define DOUT 10
#define CLK 11
#define BUZZER 1
#define BTN_PIN A5

// WiFi Produksi
// const char *ssid = "Guest_Prod";
// const char *password = "nihonseikiid123";


// WiFi QC
const char *ssid     = "Guest_QC";
const char *password = "nihonseikiid12345";



// NTP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200);

MCUFRIEND_kbv tft;
HX711 scale;
SoftwareSerial PRINTER(RX_PIN, TX_PIN);  // RX, TX

int NYALA = HIGH;
int MATI = LOW;
float CALIBRATION_WEIGHT = 200;
float calibration_factor = 58.67;
const int EEPROM_ADDRESS = 0;
unsigned long buttonPressStartTime = 0;
bool buttonPressed = false;
float BATAS_ATAS = 7.11;
float BATAS_BAWAH = 6.9;
// float BATAS_ATAS = 0.230;
// float BATAS_BAWAH = 0.180;
float previousWeight = 0.0;
const float tolerance = 0.005;
float prevWeightKg = 0.0;
bool locked = false;
unsigned long placementTime = 0;
uint16_t currentBgColor = TFT_BLACK;
String displayTgl;
unsigned long lastWifiCheckTime = 0;
const unsigned long wifiCheckInterval = 30000;  // 30 detik


String getFormattedDateTime(bool includeDate = true, bool includeTime = true) {
  if (WiFi.status() == WL_CONNECTED && testInternetConnection()) {
    // Jika terhubung ke WiFi dan ada koneksi internet, dapatkan waktu saat ini
    timeClient.update();
    unsigned long rawTime = timeClient.getEpochTime();  // Sesuaikan dengan zona waktu Anda
    time_t t = rawTime;

    String result = "";

    if (includeDate) {
      int tgl = day(t);
      String tglStr = tgl < 10 ? "0" + String(tgl) : String(tgl);

      String bulan;
      switch (month(t)) {
        case 1:
          bulan = "Jan";
          break;
        case 2:
          bulan = "Feb";
          break;
        case 3:
          bulan = "Mar";
          break;
        case 4:
          bulan = "Apr";
          break;
        case 5:
          bulan = "Mei";
          break;
        case 6:
          bulan = "Jun";
          break;
        case 7:
          bulan = "Jul";
          break;
        case 8:
          bulan = "Agu";
          break;
        case 9:
          bulan = "Sep";
          break;
        case 10:
          bulan = "Okt";
          break;
        case 11:
          bulan = "Nov";
          break;
        case 12:
          bulan = "Des";
          break;
      }

      int thn = year(t);
      String thnStr = String(thn);

      result += tglStr + "-" + bulan + "-" + thnStr + " ";
    }

    if (includeTime) {
      int jam = hour(t);
      String jamStr = jam < 10 ? "0" + String(jam) : String(jam);
      int menit = minute(t);
      String menitStr = menit < 10 ? "0" + String(menit) : String(menit);
      int detik = second(t);
      String detikStr = detik < 10 ? "0" + String(detik) : String(detik);

      result += jamStr + ":" + menitStr + ":" + detikStr;
    }

    return result;
  } else {
    // Jika tidak terhubung ke WiFi atau tidak ada koneksi internet, kembalikan string kosong
    return " ";
  }
}

void calibrateScale() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(105, 10);
  tft.setTextSize(2);
  tft.print("Mengkalibrasi");
  delay(500);
  tft.setCursor(80, 30);
  tft.print("Kosongkan Timbangan");
  countdown(4000);  // 5 seconds countdown
  scale.tare();

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(45, 40);
  tft.print("Tempatkan Benda ");
  tft.print(CALIBRATION_WEIGHT);
  tft.print(" gram");
  delay(7000);  // 3 seconds

  tft.setCursor(110, 60);
  tft.print("Sedang Proses");
  long reading = scale.get_value(10);
  calibration_factor = reading / CALIBRATION_WEIGHT;
  scale.set_scale(calibration_factor);
  // Simpan nilai calibration_factor ke EEPROM
  EEPROM.put(EEPROM_ADDRESS, calibration_factor);
  delay(5000);  // 6 seconds countdown

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(80, 30);
  tft.fillScreen(TFT_GREEN);
  tft.setCursor(80, 30);
  tft.setTextColor(TFT_BLACK);
  tft.print("Kalibrasi Berhasil");
  delay(1000);
}

void countdown(int ms) {
  for (int i = ms / 1000; i >= 0; i--) {
    tft.setCursor(60, 60);                     // Adjust the position as needed
    tft.fillRect(60, 60, 240, 20, TFT_BLACK);  // Clear the previous countdown text
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.print("Waktu tersisa: ");
    tft.print(i);
    tft.print(" detik");
    delay(1000);
  }
  delay(1000);
  tft.fillScreen(TFT_BLACK);
}

void tareScale() {
  tft.setCursor(120, 170);
  tft.setTextSize(2);
  tft.print("Tare Scaling");
  scale.tare();
  delay(1000);
  tft.setCursor(165, 190);
  tft.print("Done");
  delay(1000);
}

void printWeight() {
  tft.setCursor(140, 170);
  tft.setTextSize(2);
  tft.print("Printing");



  PRINTER.print("^XA");     // Start of label format
  PRINTER.print("^PW380");  // Set label width in dots (2.8 cm * 203 dpi = 560 dots)
  PRINTER.print("^LL800");  // Set label length in dots (5.3 cm * 203 dpi = 800 dots)

  // First line
  PRINTER.print("^FO29,30");
  PRINTER.print("^A0N,27,27");
  PRINTER.print("^FDPT NIHON SEIKI INDONESIA^FS");

  // Second line
  PRINTER.print("^FO45,100");
  PRINTER.print("^A0N,40,40");
  PRINTER.print("^FDWeight: ");
  PRINTER.print(prevWeightKg, 3);
  PRINTER.print(" Kg^FS");

  // Third line
  PRINTER.print("^FO50,175");
  PRINTER.print("^A2N,25,10");
  PRINTER.print("^FD" + getFormattedDateTime() + "^FS");

  PRINTER.print("^XZ");  // End of label format
  delay(1500);
  tft.setCursor(165, 190);
  tft.setTextSize(2);
  tft.print("Done");
  currentBgColor = TFT_GREEN;                       // Update current background color
  delay(1000);                                      // Wait for 1 second before clearing the messages
  tft.fillRect(140, 170, 180, 40, currentBgColor);  // Adjust the coordinates and size of the rectangle to clear the text area
  delay(500);
}

bool testInternetConnection() {
  WiFiClient client;
  unsigned long startTime = millis();  // Mengambil waktu awal koneksi
  while (!client.connect("8.8.8.8", 53)) {
    // Cek jika waktu telah melebihi 5 detik
    if (millis() - startTime > 5000) {
      return false;  // Koneksi gagal setelah 5 detik timeout
    }
  }
  return true;  // Koneksi berhasil
}

void reconnectWiFi() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 10);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.print("Reconnecting to WiFi...");
  WiFi.begin(ssid, password);  // Ganti dengan nama dan kata sandi WiFi Anda

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 5) {
    delay(500);
    tft.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 10);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.print("WiFi reconnected!");
    displayTgl = getFormattedDateTime(true, false);
    delay(2000);
  } else {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(50, 40);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.print("Failed reconnect to WiFi.");
    delay(1000);
    tft.fillScreen(TFT_BLACK);
    displayTgl = getFormattedDateTime(true, false);
    delay(2000);
  }
}

void updateLCD(float weightGrams) {
  // Ubah gram menjadi kilogram
  float weightKg = weightGrams / 1000.0;  // 1 kg = 1000 gram

  if (abs(weightKg - prevWeightKg) >= tolerance) {
    weightKg = round(weightKg * 10000) / 10000.0;
    if (weightKg < 0.02) {
      weightKg = 0;
      digitalWrite(BUZZER, LOW);
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE);
      currentBgColor = TFT_BLACK;  // Update current background color
    } else if (weightKg > 0.198 && weightKg < 0.202) {
      weightKg = 0.200;
      digitalWrite(BUZZER, LOW);
      tft.fillScreen(TFT_BLUE);
      tft.setTextColor(TFT_WHITE);
      tft.setCursor(100, 60);
      tft.setTextSize(2);
      tft.print("Verifikasi Berhasil");
      currentBgColor = TFT_BLUE;  // Update current background color
    } else if (weightKg < BATAS_BAWAH) {
      digitalWrite(BUZZER, HIGH);
      delay(500);
      digitalWrite(BUZZER, LOW);
      tft.fillScreen(TFT_RED);
      tft.setTextColor(TFT_WHITE);
      currentBgColor = TFT_RED;  // Update current background color
    } else if (weightKg >= BATAS_BAWAH && weightKg <= BATAS_ATAS) {
      digitalWrite(BUZZER, LOW);
      tft.fillScreen(TFT_GREEN);
      tft.setTextColor(TFT_BLACK);
      currentBgColor = TFT_GREEN;  // Update current background color
    } else if (weightKg > BATAS_ATAS) {
      digitalWrite(BUZZER, HIGH);
      delay(500);
      digitalWrite(BUZZER, LOW);
      tft.fillScreen(TFT_RED);
      tft.setTextColor(TFT_WHITE);
      currentBgColor = TFT_RED;  // Update current background color
    }

    tft.setCursor(120, 10);
    tft.setTextSize(2);
    tft.print("IT & MTC DEPT");
    tft.setCursor(100, 40);
    tft.setTextSize(1);
    tft.print("Min: ");
    tft.print(BATAS_BAWAH);
    tft.print(" Kg");

    tft.setCursor(210, 40);
    tft.setTextSize(1);
    tft.print("Max: ");
    tft.print(BATAS_ATAS);
    tft.print(" Kg");

    tft.setCursor(80, 220);
    tft.setTextSize(2);
    tft.print("NIHON SEIKI INDONESIA");
    tft.setCursor(60, 100);
    tft.setTextSize(6);
    tft.print(weightKg, 3);  // Menampilkan 3 digit di belakang koma
    tft.print(" Kg");

    tft.setCursor(135, 200);
    tft.setTextSize(2);
    // displayTgl = getFormattedDateTime(true, false);
    tft.print(displayTgl);
    // tft.print(displayTgl);  // Hanya mendapatkan tanggal;
    // Perbarui nilai berat sebelumnya
    prevWeightKg = weightKg;
  }
}

void setup() {
  // Serial.begin(9600);  // Inisialisasi serial untuk debug
  uint16_t ID = tft.readID();
  tft.begin(ID);
  // tft.fillScreen(TFT_BLACK);
  // tft.drawBitmap(60, 100, nsi_logo, 120, 200, TFT_WHITE);
  // delay(2000);
  tft.fillScreen(TFT_BLACK);
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
  // WiFi initialization

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(10, 10);
  tft.print("Connecting to ");
  tft.setCursor(10, 40);
  tft.print(ssid);
  delay(1000);
  WiFi.begin(ssid, password);

  if (WiFi.status() == WL_CONNECTED) {
    tft.setCursor(10, 70);
    tft.setTextSize(3);
    tft.print("Connected");

    if (testInternetConnection()) {
      tft.setCursor(10, 100);
      tft.setTextSize(3);
      tft.print("Internet OK");
    } else {
      tft.setCursor(10, 100);
      tft.setTextSize(3);
      tft.print("No Internet");
    }

    timeClient.begin();  // Initialize the timeClient

  } else {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 50);
    tft.setTextSize(3);
    tft.print("WiFi not connected");
  }
  delay(2000);

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(120, 10);
  tft.setTextSize(2);
  tft.print("IT & MTC DEPT");
  tft.setCursor(80, 220);
  tft.setTextSize(2);
  tft.print("NIHON SEIKI INDONESIA");
  tft.setCursor(60, 100);
  tft.setTextSize(6);
  tft.print("0.000 Kg");
  tft.setCursor(100, 40);
  tft.setTextSize(1);
  tft.print("Min: ");
  tft.print(BATAS_BAWAH);
  tft.print(" Kg");

  tft.setCursor(210, 40);
  tft.setTextSize(1);
  tft.print("Max: ");
  tft.print(BATAS_ATAS);
  tft.print(" Kg");

  tft.setCursor(135, 200);
  tft.setTextSize(2);
  displayTgl = getFormattedDateTime(true, false);
  tft.print(displayTgl);
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

      if (pressDuration >= 8000) {
        calibrateScale();
      } else if (pressDuration >= 5000 && pressDuration < 8000) {
        tareScale();
      } else if (pressDuration > 0 && pressDuration < 5000) {
        float weightGram = scale.get_units(5);
        float weightKg = weightGram / 1000.0;  // 1 kg = 1000 gram
        weightKg = round(weightKg * 10000) / 10000.0;
        if (weightKg >= BATAS_BAWAH && weightKg <= BATAS_ATAS) {
          printWeight();
        } else {
          tft.setCursor(70, 170);
          tft.setTextSize(2);
          tft.print("Tidak Dapat Di Print");
          delay(1000);
          tft.fillRect(70, 170, 240, 30, currentBgColor);
        }
      }
    }
  }

  // Baca berat saat ini
  float currentWeight = scale.get_units(5);

  // Bandingkan dengan nilai berat sebelumnya
  if (abs(currentWeight - previousWeight) >= tolerance) {
    // Perubahan berat lebih besar atau sama dengan toleransi, update tampilan LCD
    updateLCD(currentWeight);

    previousWeight = currentWeight;
  }

  if (!locked && scale.is_ready()) {
    float currentWeight = scale.get_units(10);
    if (millis() - placementTime > 10000) {
      if (abs(currentWeight - previousWeight) > tolerance) {
        locked = true;
        previousWeight = currentWeight;
      }
    }
  }
  if (locked) {
    if (prevWeightKg < 0.02) {
      digitalWrite(BUZZER, LOW);
    } else if (prevWeightKg > 0.198 && prevWeightKg < 0.202) {
      digitalWrite(BUZZER, LOW);
    } else if (prevWeightKg < BATAS_BAWAH) {
      digitalWrite(BUZZER, HIGH);
      delay(500);
      digitalWrite(BUZZER, LOW);
    } else if (prevWeightKg >= BATAS_BAWAH && prevWeightKg <= BATAS_ATAS) {
      digitalWrite(BUZZER, LOW);
    } else if (prevWeightKg > BATAS_ATAS) {
      digitalWrite(BUZZER, HIGH);
      delay(500);
      digitalWrite(BUZZER, LOW);
    }
  }

  // Pengecekan status koneksi WiFi setiap 30 detik

  if (WiFi.status() != WL_CONNECTED) {

    // tft.setCursor(10, 10);
    // tft.setTextSize(1);
    // tft.print("WiFi Disconnected");
    tft.fillRect(10, 10, 20, 20, TFT_RED);
    if (millis() - lastWifiCheckTime >= wifiCheckInterval) {
      lastWifiCheckTime = millis();
      // Memanggil fungsi untuk menyambungkan ulang WiFi jika terputus
      reconnectWiFi();
    }
  }
}

