#include <Wire.h>

#define SensorPin 0 // Connect the LM35 temperature sensor output to pin A0 (Analog 0). This pin is mapped to ADC channel 0.

unsigned long avgValue;    // Pointer to store the average value of the sensor feedback
float voltage, ntu; // Variables to store the voltage and calculated temperature
int buf[10], temp;         // Buffer to store sensor readings and temporary variable
unsigned long previousMillis = 0; // Variable to store previous time for sampling interval
const unsigned long samplingInterval = 100; // Sampling interval in milliseconds
int analog_value = 0;      // Variable to store the current analog reading


void setupADC() {
  // Set reference voltage to AVcc and select channel 0 (A0 initially)
  ADMUX = (1 << REFS0); // Reference voltage = AVcc, channel = 0 by default
  
  // Enable ADC and set prescaler to 128 for 16MHz Arduino boards (125 kHz ADC clock)
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}


void setupSerial() {
  Serial.begin(115200); // Initialize Serial communication for debugging
}

void sendSerialWithID(uint16_t value) {
  Wire.beginTransmission(1); // Begin I²C transmission to slave device at address 1
  Wire.write(value);
  Wire.endTransmission();    // End transmission
}

void setup() {
  Wire.begin();     // Initialize I²C communication
  setupADC();       // Initialize ADC
  setupSerial();    // Initialize Serial communication (optional)
}


// Function to read analog values directly from a specific ADC channel
uint16_t analogReadDirect(uint8_t channel) {
  ADMUX = (ADMUX & 0xF8) | (channel & 0x07); // Select the ADC channel
  delayMicroseconds(10); // Stabilize ADC
  ADCSRA |= (1 << ADSC); // Start ADC conversion
  while (ADCSRA & (1 << ADSC)); // Wait for conversion to finish
  return ADC; // Return the 10-bit ADC result
}

void loop() {
  unsigned long currentMillis = millis(); // Get the current system time

  if (currentMillis - previousMillis >= samplingInterval) {
    previousMillis = currentMillis; // Update the previousMillis for timing

    // Collect 10 samples for averaging
    for (int i = 0; i < 10; i++) {
      analog_value = analogReadDirect(SensorPin); // Read the sensor value from pin A0
      delay(10); // Delay for stabilization between reads
    }



     // Convert raw ADC value to voltage (assuming 5V system)
    long voltage = analog_value * (5.0 / 1023.0);
    
    // Calculate turbidity in NTU (example calibration formula)
    long turbidity = -1333.33 * voltage + 5333.33;
    // Send turbidity value over I²C
    sendSerialWithID(turbidity);
    Serial.println(turbidity);

  }

  delay(1000);
}

