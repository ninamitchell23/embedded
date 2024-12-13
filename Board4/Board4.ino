
// Pin Connections and Setup:
// - ALERT_LED: Connect to Pin 9 (PB1)
// - BOARD_2_LED: Connect to Pin 8 (PB0)
// - BOARD_3_LED: Connect to Pin 7 (PD7)
// - BUZZER: Connect to Pin 4 (PD4)
// 
// Register Configuration in Setup:
// - DDRB |= (1 << DDB1): Sets ALERT_LED as an output (Pin 9 -> PB1).
// - DDRB |= (1 << DDB0): Sets BOARD_2_LED as an output (Pin 8 -> PB0).
// - DDRD |= (1 << DDD7): Sets BOARD_3_LED as an output (Pin 7 -> PD7).
// - DDRD |= (1 << DDD4): Sets BUZZER as an output (Pin 4 -> PD4).
//
// Operation Overview:
// - ALERT_LED and BUZZER are used for signaling alerts.
// - BOARD_2_LED and BOARD_3_LED indicate data readiness from respective boards.
//
// SoftwareSerial Configuration:
// - b2 (TMP sensor) is assigned to Pins 10 (RX) and 11 (TX).
// - Communication is set to 9600 baud.
//
// Thresholds for Sensor Alerts:
// - NTU_THRESHOLD: Turbidity alert level.
// - TEMP_THRESHOLD: Temperature alert level.
//
// EEPROM Usage:
// - Address 0: Stores NTU alert status.
// - Address 1: Stores temperature alert status.
//
// Alert Timing:
// - Alert duration for buzzer and LED is 400ms.
// - LED indicators for sensors reset after 100ms.

#include "SoftwareSerial.h"
#include <EEPROM.h>
#include <Wire.h>
// Define pins for LEDs and buzzer
#define ALERT_LED 9
#define BOARD_2_LED 8
#define BOARD_3_LED 7
#define BUZZER 4

// Thresholds for NTU and temperature
const float NTU_THRESHOLD = 50.0;    //  threshold for turbidity
const float TEMP_THRESHOLD = 30.0;   // threshold for temperature

String incomingData2 = ""; 
String incomingData3 = ""; // From b3 (9600 baud, for temperature)

bool dataReady_board_2 = false;
bool dataReady_board_3 = false;

unsigned long startb3 = 0;
unsigned long startb2 = 0;
unsigned long led_ph_time = 0;
unsigned long led_tmp_time = 0;
unsigned long led_tb_time = 0;
unsigned long alertStartTime = 0;
bool alertActive = false;
bool ledtmp = false;
bool ledtb = false;
bool ledph = false;

SoftwareSerial b2(10, 11);  // TMP sensor

void setup() {
  // Set up serial communications
  Serial.begin(115200);
  b2.begin(115200);

  // Configure LED and buzzer pins as outputs using registers
  DDRB |= (1 << DDB1); // Configure PB1 (Pin 9 -> ALERT_LED) as output
  DDRB |= (1 << DDB0); // Configure PB0 (Pin 8 -> BOARD_2_LED) as output
  DDRD |= (1 << DDD7); // Configure PD7 (Pin 7 -> BOARD_3_LED) as output
  DDRD |= (1 << DDD4); // Configure PD4 (Pin 4 -> BUZZER) as output

  // Initialize timing variables
  led_tb_time = millis();
  led_tmp_time = millis();
  Wire.begin(1);
  Wire.onReceive(receiveEvent);

  
}


void receiveEvent(int num){

     incomingData2 = Wire.read();

      dataReady_board_2 = true;

}



void loop() {
  startb2 = millis();
  readBoard_3_Data();

  // Handle Board 2 Data (NTU)
  if (dataReady_board_2) {
    PORTB |= (1 << PORTB0); // Turn BOARD_2_LED (Pin 8) ON
    processBoard_2_Data();
    ledtb = true;
  }

  // Handle Board 3 Data (Temperature)
  if (dataReady_board_3) {
    PORTD |= (1 << PORTD7); // Turn BOARD_3_LED (Pin 7) ON
    processBoard_3_Data();
    ledtmp = true;
  }

  // Turn off BOARD_3_LED after 200ms if it was turned on
  if (millis() - led_tmp_time > 1000 && ledtmp) {
    PORTD &= ~(1 << PORTD7); // Turn BOARD_3_LED (Pin 7) OFF
    ledtmp = false;
    led_tmp_time = millis();
  }

  // Turn off BOARD_2_LED after 200ms if it was turned on
  if (millis() - led_tb_time > 1000 && ledtb) {
    PORTB &= ~(1 << PORTB0); // Turn BOARD_2_LED (Pin 8) OFF
    led_tb_time = millis();
    ledtb = false;
  }

  // Handle buzzer alerts without delay
  handleAlert();
}





void readBoard_3_Data() {
  while (Serial.available() > 0) {
    char incomingChar = Serial.read();
    if (incomingChar == '\n') {
      dataReady_board_3 = true;
      break;
    }
    incomingData3 += incomingChar;
  }
}

void processBoard_2_Data() {
  long turbidity = incomingData2.toFloat();
  String output = "TB:" + String(turbidity);
  Serial.println(output);

  if (turbidity > NTU_THRESHOLD) {
    activateAlert("TB:ALARM");
    EEPROM.write(0, 1); // Save NTU alert to EEPROM
     Serial.println("TB: ALERT");
  }

  incomingData2 = "";
  dataReady_board_2 = false;
}

void processBoard_3_Data() {
  float temperature = incomingData3.toFloat();
  String output = "Temp:" + String(temperature);
  Serial.println(output);
  if (temperature > TEMP_THRESHOLD) {
    activateAlert("Temp:ALARM");
    EEPROM.write(1, 1); // Save temperature alert to EEPROM
    Serial.println("Temp:ALERT");
  }
  incomingData3 = "";
  dataReady_board_3 = false;
}

void activateAlert(String str) {
  alertActive = true;
  alertStartTime = millis();
  PORTD |= (1 << PORTD4); // Turn BUZZER (Pin 4) ON
  PORTB |= (1 << PORTB1); // Turn ALERT_LED (Pin 9) ON
}

void handleAlert() {
  if (alertActive && (millis() - alertStartTime >= 400)) {
    PORTD &= ~(1 << PORTD4); // Turn BUZZER (Pin 4) OFF
    PORTB &= ~(1 << PORTB1); // Turn ALERT_LED (Pin 9) OFF
    alertActive = false;
  }
}
