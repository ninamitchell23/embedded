#include<Wire.h>
int sensorPin = A0;  
int digitalPin = 8;  
int sensorValue = 0;  
int digitalValue = 0;  
float voltage = 0;
float turbidity = 0;

float previousTurbidity = -1;  
int previousDigitalValue = -1;  

void setup() {
  pinMode(digitalPin, INPUT);  
  Serial.begin(115200);  
  Wire.begin(); 
}


void sendSerialWithID(float value) {
   float valueToSend = value;
  
  Wire.beginTransmission(1); // Address of the slave device
  byte* floatBytes = (byte*)&valueToSend; // Cast float to byte array
  
  for (int i = 0; i < sizeof(float); i++) {
    Wire.write(floatBytes[i]); // Send each byte
  }
  Wire.endTransmission();
  
}


void loop() {
  
  sensorValue = analogRead(sensorPin);
  voltage = sensorValue * (5.0 / 1023.0);  
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

    previousTurbidity = turbidity;
  }


  

  delay(100); 
}