/*
  AVR Registers Explanation:
  1. ADMUX (ADC Multiplexer Selection Register):
     - Bits [7:6]: Reference voltage selection (REFS1:REFS0).
       Setting REFS0 to 1 selects AVcc as the reference voltage.
     - Bits [3:0]: Analog channel selection.
       These bits choose the ADC channel (ADC0 to ADC7). For example, connecting to pin A0 corresponds to ADC channel 0.

  2. ADCSRA (ADC Control and Status Register A):
     - ADEN: ADC Enable. Set to 1 to enable the ADC.
     - ADSC: ADC Start Conversion. Writing 1 starts a conversion.
     - ADPS2:ADPS0: ADC Prescaler bits to divide system clock.
       Prescaler of 128 is set for 16 MHz systems for stable readings.

  3. ADC (Analog-to-Digital Converter Data Register):
     - A 10-bit register that holds the conversion result.

  4. millis(): Provides system uptime in milliseconds.
     Useful for time-based operations.
*/

#define SensorPin 0 // Connect the LM35 temperature sensor output to pin A0 (Analog 0). This pin is mapped to ADC channel 0.

unsigned long avgValue;    // Pointer to store the average value of the sensor feedback
float voltage, temperature; // Variables to store the voltage and calculated temperature
int buf[10], temp;         // Buffer to store sensor readings and temporary variable
unsigned long previousMillis = 0; // Variable to store previous time for sampling interval
const unsigned long samplingInterval = 100; // Sampling interval in milliseconds
int analog_value = 0;      // Variable to store the current analog reading

void setup() {
  Serial.begin(115200);  // Initialize serial communication at 9600 baud rate

  // Set up the ADC
  ADMUX = (1 << REFS0); // Set AVcc as the reference voltage and select ADC0 (Pin A0)
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC with a prescaler of 128
}

// Function to calculate temperature based on LM35 output
void calculateTemperature(float *voltage, float *temperature, unsigned long avgValue) {
  *voltage = (float)avgValue * 5.0 / 1024; // Convert ADC value to voltage
  *temperature = *voltage * 100; // LM35 outputs 10mV per °C (5V = 500°C, 1V = 100°C)
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

    // Calculate the temperature from the average analog value
    calculateTemperature(&voltage, &temperature, analog_value);

    // Print the temperature in Celsius (2 decimal places)
    Serial.println(temperature, 2);
  }

  delay(1500);
}
