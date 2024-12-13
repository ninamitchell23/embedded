/*
  AVR Registers Explanation:
  1. ADMUX (ADC Multiplexer Selection Register):
     - Bits [7:6]: Reference voltage selection (REFS1:REFS0).
       Setting REFS0 to 1 selects AVcc as the reference voltage.
     - Bits [3:0]: Analog channel selection.
       These bits choose the ADC channel (ADC0 to ADC7).

  2. ADCSRA (ADC Control and Status Register A):
     - ADEN: ADC Enable. Set to 1 to enable the ADC.
     - ADSC: ADC Start Conversion. Writing 1 starts a conversion.
     - ADPS2:ADPS0: ADC Prescaler bits to divide system clock.
       Prescaler of 128 is set for 16 MHz systems for stable readings.

  3. ADC (Analog-to-Digital Converter Data Register):
     - A 10-bit register that holds the conversion result.

  4. millis(): Provides system uptime in milliseconds.
     Useful for time-based operations.

  5. _BV(x): A macro that returns a byte value with bit x set.
     Commonly used in register bit manipulations.
*/

#define SensorPin 0 // Connect the TB meter output sensor to pin A0 (Analog 0). This pin is mapped to ADC channel 0.


unsigned long avgValue;    // Pointer to store the average value of the sensor feedback
float voltage, turbidity;   // Variables to store the voltage and calculated turbidity
int buf[10], temp;         // Buffer to store sensor readings and temporary variable
unsigned long previousMillis = 0; // Variable to store previous time for sampling interval
const unsigned long samplingInterval = 100; // Sampling interval in milliseconds

void setup() {
  Serial.begin(9600);  // Initialize serial communication at 9600 baud rate

  // Set up the ADC
  ADMUX = (1 << REFS0); // Set AVcc as the reference voltage and select ADC0
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC with a prescaler of 128
}

uint16_t analogReadDirect(uint8_t channel) {
  // Select the channel using bitwise operations to set the ADC channel
  ADMUX = (ADMUX & 0xF8) | (channel & 0x07);

  // Start the conversion by setting ADSC bit
  ADCSRA |= (1 << ADSC);

  // Wait for the conversion to complete by checking the ADSC bit
  while (ADCSRA & (1 << ADSC));

  // Return the ADC result from the ADC register
  return ADC;
}

void processSamples(int *buffer, unsigned long *average) {
  // Sort the samples from smallest to largest
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buffer[i] > buffer[j]) {
        // Swap the values to sort the array
        int temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
      }
    }
  }

  // Calculate the average of the middle 6 values to minimize noise
  *average = 0;
  for (int i = 2; i < 8; i++) {
    *average += buffer[i];
  }
  *average /= 6;  // Take the average of the 6 middle values
}

void calculateTurbidity(float *voltage, float *turbidity, unsigned long avgValue) {
  // Convert the average ADC value to voltage (5V reference)
  *voltage = (float)avgValue * 5.0 / 1024;
  
  // Calculate turbidity using the voltage (based on sensor's characteristics)
  *turbidity = -1333.33 * (*voltage) + 5333.33;
  
  // Ensure that turbidity does not become negative
  *turbidity = max(*turbidity, 0); // Clamp negative turbidity to 0
}

void loop() {
  unsigned long currentMillis = millis();  // Get the current system time

  // If enough time has passed for the next sample (based on sampling interval)
  if (currentMillis - previousMillis >= samplingInterval) {
    previousMillis = currentMillis;  // Update the previousMillis time

    // Collect 10 samples from the sensor
    for (int i = 0; i < 10; i++) {
      buf[i] = analogReadDirect(SensorPin);  // Read sensor value
      delay(10); // Small delay between readings
    }

    // Process the collected samples and calculate the average value
    processSamples(buf, &avgValue);

    // Calculate turbidity based on the average ADC value
    calculateTurbidity(&voltage, &turbidity, avgValue);

    // Print the calculated turbidity to the serial monitor
    Serial.println(turbidity);
  }
}
