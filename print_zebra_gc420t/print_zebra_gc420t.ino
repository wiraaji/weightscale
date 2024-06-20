#include <SoftwareSerial.h>
#define TX_PIN 13  // Connect to printer RX
#define RX_PIN 12  // Connect to printer TX
SoftwareSerial mySerial(RX_PIN, TX_PIN);  // RX, TX

void setup() {
  // Set the data rate for the SoftwareSerial port
  mySerial.begin(9600);

  // Print command for Zebra printer using ZPL
  mySerial.print("^XA"); // Start of label format
  mySerial.print("^PW380"); // Set label width in dots (2.8 cm * 203 dpi = 560 dots)
  mySerial.print("^LL800"); // Set label length in dots (5.3 cm * 203 dpi = 800 dots)
  mySerial.print("^FO20,20"); // Set origin of text
  
  // Set font size for the first line
  mySerial.print("^A0N,27,27"); // Set font: Font A, size 30
  
  // Print the first line
  mySerial.print("^FDPT NIHON SEIKI INDONESIA^FS"); // Print text
  
  // Second line (adjusted position and font)
  mySerial.print("^FO20,100"); // Set origin of second line
  mySerial.print("^A0N,27,27"); // Set font for second line (Font A, size 30)
  mySerial.print("^FDWeight: 500g^FS"); // Print second line text
  
  mySerial.print("^XZ"); // End of label format
}

void loop() {
  
}
