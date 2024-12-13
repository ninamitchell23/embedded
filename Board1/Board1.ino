#include <LiquidCrystal.h>

// Pin Mapping
const int rs = 12, en = 11, d4 = 4, d5 = 5, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Variables for data and flags
String incomingData = "";  
String data_temp = "Temp: 0*c";
String data_tb = "TB: 0 NTU";
bool dataReady = false;

void setup() {
  Serial.begin(115200);

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.setCursor(2, 0);
  lcd.print("INITIALISING");
  lcd.setCursor(2, 1);
  lcd.print("SYSTEM");
  delay(100); // Show the initializing message for 1 second
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("INITIALISED!");
  delay(100); // Show the initialized message for half a second
  lcd.clear();
  lcd.print("WAITING....");
  delay(50);
  lcd.clear();
}

void loop() {
  // Display current data
  lcd.setCursor(1, 0);
  lcd.print(data_tb); // Display turbidity data
  lcd.setCursor(1, 1);
  lcd.print(data_temp); // Display temperature data

  // Read incoming data
  while (Serial.available() > 0) {
    char incomingChar = Serial.read();
    if (incomingChar == '\n') {
      dataReady = true; // Mark data as ready when newline is received
      break;
    }
    incomingData += incomingChar; // Append received character
  }

  // Process incoming data
  if (dataReady) {
    if (incomingData.startsWith("Temp:")) {
      data_temp = incomingData + "*c"; // Add temperature unit
    } else if (incomingData.startsWith("TB:")) {
      data_tb = incomingData + " NTU"; // Add turbidity unit
    }

    incomingData = ""; // Clear incoming data buffer
    dataReady = false; // Reset data ready flag
  }

  delay(100); // Wait for 1 second before the next update
}
