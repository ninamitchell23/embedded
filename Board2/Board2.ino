// Include the Wire library for I2C communication
#include<Wire.h>

// Define the analog sensor pin
int sensorPin = A0;  

// Define the digital pin for additional functionality
int digitalPin = 8;  

// Variable to store the raw sensor value
int sensorValue = 0;  

// Variables to store calculated voltage and turbidity
float voltage = 0;
float turbidity = 0;

// Setup function to initialize serial communication and I2C
void setup() {
  
  Serial.begin(115200);  // Begin serial communication at 115200 baud rate
  Wire.begin(); // Initialize the I2C communication
}

// Function to send a float value with an ID over I2C
void sendSerialWithID(float value) {
   float valueToSend = value; // Prepare the value to send
  
  Wire.beginTransmission(1); // Address of the slave device
  byte* floatBytes = (byte*)&valueToSend; // Cast float to byte array
  
  for (int i = 0; i < sizeof(float); i++) {
    Wire.write(floatBytes[i]); // Send each byte of the float value
  }
  Wire.endTransmission(); // End the I2C transmission
  
  // Configure ADC for analog to digital conversion
  ADMUX = B01000000;   // Use AVcc as reference, select channel A0
  ADCSRA = B10000111;  // Enable ADC, set prescaler to 128
}


void loop() {
  
 // Start ADC conversion
  ADCSRA |= B01000000;

  // Wait for conversion to complete
  while (bit_is_set(ADCSRA, ADSC));

  // Read ADC result (combine ADCL and ADCH)
  int val = ADCL | (ADCH << 8);
  
   // Convert ADC value to voltage
  voltage = (val / 1023.0) * 5.0; 
  turbidity = -1333.33 * voltage + 5333.33;
  if (turbidity < 0) {
    turbidity = 0;
  }

  if (true) {
    Serial.print("Analog Voltage: ");
    Serial.print(voltage);
    Serial.print(" V, ");
    Serial.print("Turbidity: ");
    Serial.print(turbidity);
    Serial.println(" NTU");
    sendSerialWithID(turbidity);

 
  }


  

  delay(100); 
}
