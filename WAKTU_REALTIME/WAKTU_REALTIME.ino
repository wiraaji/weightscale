
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h> 
#include <TimeLib.h> 


const char *ssid     = "Guest_Prod";
const char *password = "nihonseikiid123";

// const char *ssid = "trial";
// const char *password = "12345678";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

int last_second;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  timeClient.begin();

}

// void loop() {
//   timeClient.update();
//   //Serial.println(timeClient.getFormattedTime());
//   //Serial.println(timeClient.getEpochTime());
//   unsigned long rawTime = timeClient.getEpochTime() + 25200;

//   if (rawTime != last_second){
//   time_t t = rawTime;
 
//   int jam = hour(t);
//   String jamStr = jam < 10 ? "0" + String(jam) : String(jam);
//   int menit = minute(t);
//   String menitStr = menit < 10 ? "0" + String(menit) : String(menit);
//   int detik = second(t);
//   String detikStr = detik < 10 ? "0" + String(detik) : String(detik);
    
//   String hari;
//   switch (weekday(t)){
//     case 1 :
//       hari = "Minggu";
//       break;
//     case 2 :
//       hari = "Senin";
//       break;
//     case 3 :
//       hari = "Selasa";
//       break;
//     case 4 :
//       hari = "Rabu";
//       break;
//     case 5 :
//       hari = "Kamis";
//       break;
//     case 6 :
//       hari = "Jumat";
//       break;
//     case 7 :
//       hari = "Sabtu";
//       break;
//     }
//   int tgl = day(t);
//   String tglStr = tgl < 10 ? "0" + String(tgl) : String(tgl);
//   int bln = month(t);
//   String blnStr = bln < 10 ? "0" + String(bln) : String(bln);
//   int thn = year(t);
//   String thnStr = String(thn);

//   String tanggal = hari + " " + tglStr + "/" + blnStr + "/" + thnStr + " ";
//   String waktu = jamStr + ":" + menitStr + ":" + detikStr;

//   Serial.print(tanggal);
//   Serial.println(waktu);

//   delay(100);
//   last_second = rawTime;
//   }
// }

void loop() {
  timeClient.update();

  Serial.println(timeClient.getFormattedTime());

  delay(1000);
}
