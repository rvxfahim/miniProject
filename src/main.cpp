#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <ArduinoJson.h>
#include <math.h>
// Define the pins for the cooling power input
const int coolingPowerPin = A0;

// Define the constants for the thermal model
const float wallThermalResistance = 0.005; // in K/W
const float roomVolume = 20; // in m^3
const float roomHeatCapacity = 1000; // in J/K
const float roomInitialTemperature = 30; // in degC
const float outsideTemperature = 30; // in degC

// Define the variables for the thermal model
float insideTemperature = roomInitialTemperature;
float coolingPower = 0;

// Define the task handles
TaskHandle_t simulationTaskHandle;
TaskHandle_t printTaskHandle;

// Define the task functions
void simulationTask(void *pvParameters);
void printTask(void *pvParameters);

void setup() {
  // Initialize the serial communication
  Serial.begin(115200);
  pinMode(A0,INPUT);
  Serial.print(F("configTICK_RATE_HZ:"));
  Serial.println(configTICK_RATE_HZ);
  Serial.print(F("portTICK_PERIOD_MS:"));
  Serial.println(portTICK_PERIOD_MS);
  Serial.print(F("freeRTOS version:"));
  Serial.println(tskKERNEL_VERSION_NUMBER);
  // Create the simulation task
  xTaskCreate(simulationTask, "Simulation Task", 128, NULL, 1, &simulationTaskHandle);

  // Create the print task
  xTaskCreate(printTask, "Print Task", 512, NULL, 2, &printTaskHandle);

  // Start the scheduler
  vTaskStartScheduler();
}

void loop() {
  // This function should never be called
}

void simulationTask(void *pvParameters) {
  // Loop forever
  while (1) {
    // Read the cooling power from the input
    coolingPower = analogRead(coolingPowerPin) * 11.71875; // convert the ADC value to W
    // coolingPower = 1000;
    // Calculate the heat transfer between the inside and outside
    float heatTransfer = (outsideTemperature - insideTemperature) / wallThermalResistance;

    // If cooling power is present, subtract it from the heat transfer
    if (coolingPower > 0) {
      heatTransfer -= coolingPower;
    }

    // Calculate the change in temperature due to heat transfer
    float temperatureChange = heatTransfer / (roomVolume * roomHeatCapacity);

    // Calculate the new inside temperature
    insideTemperature += temperatureChange;

    // Delay for a short time before repeating the loop
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void printTask(void *pvParameters) {
  // Loop forever
  while (1) {
    // Create a JSON document
    StaticJsonDocument<32> doc;
    // Add key-value pairs to the document   
    doc["outT"] = round(outsideTemperature * 100.0) / 100.0;
    doc["inT"] = round(insideTemperature * 100.0) / 100.0;
    doc["watt"] = round(coolingPower * 100.0) / 100.0;
    doc["tm"] = round(millis() / 10.0) / 100.0;
    String jsonString;
    serializeJson(doc, jsonString);
    Serial.println(jsonString);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}