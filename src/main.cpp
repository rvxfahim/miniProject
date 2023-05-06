#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <ArduinoJson.h>
#include <queue.h>
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
float desiredTemperature = 0;
// Define the task handles
TaskHandle_t simulationTaskHandle;
TaskHandle_t printTaskHandle;
TaskHandle_t receiveTaskHandle;
// Define the queue handle
QueueHandle_t queueHandle;
// Define the size of the JSON buffer
const size_t JSON_BUFFER_SIZE = JSON_OBJECT_SIZE(4) + 40;
// Define the task functions
void simulationTask(void *pvParameters);
void printTask(void *pvParameters);
void receiveTask(void *pvParameters);

void setup() {
  // Initialize the serial communication
  // analogReference(EXTERNAL);
  pinMode(coolingPowerPin, INPUT);
  pinMode(9, OUTPUT);
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.setTimeout(10);
  Serial1.setTimeout(10);
  queueHandle = xQueueCreate(10, JSON_BUFFER_SIZE);
  // Serial.print(F("configTICK_RATE_HZ:"));
  // Serial.println(configTICK_RATE_HZ);
  // Serial.print(F("portTICK_PERIOD_MS:"));
  // Serial.println(portTICK_PERIOD_MS);
  // Serial.print(F("freeRTOS version:"));
  // Serial.println(tskKERNEL_VERSION_NUMBER);
  // Create the simulation task
  xTaskCreate(simulationTask, "Simulation Task", 1024, NULL, 2, &simulationTaskHandle);

  // Create the print task
  xTaskCreate(printTask, "Print Task", 512, NULL, 2, &printTaskHandle);

  // Create receive task
  xTaskCreate(receiveTask, "Receive Task", 512, NULL, 1, &receiveTaskHandle);
  // Start the scheduler
  vTaskStartScheduler();
}

void loop() {
  // This function should never be called
}

void simulationTask(void *pvParameters) {
  // Allocate a buffer for the JSON string
  char jsonStringBuffer[JSON_BUFFER_SIZE];
  // Loop forever
  while (1) {
    
    //if desired temperature is lower than inside temperature then use cooling power
    if (desiredTemperature < insideTemperature)
    {
        // Read the cooling power from the input
    coolingPower = analogRead(coolingPowerPin) * 11.71875; // convert the ADC value to W
    }
    else
    {
      coolingPower = 0;
    }
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
    StaticJsonDocument<32> doc;
    // Add key-value pairs to the document   
    doc["outT"] = round(outsideTemperature * 100.0) / 100.0;
    doc["inT"] = round(insideTemperature * 100.0) / 100.0;
    doc["watt"] = round(coolingPower * 100.0) / 100.0;
    doc["tm"] = round(millis() / 10.0) / 100.0;
    size_t jsonStringSize = measureJson(doc);
    serializeJson(doc, jsonStringBuffer, jsonStringSize + 1);
     // Send the JSON string to the output task
    xQueueSend(queueHandle, &jsonStringBuffer, portMAX_DELAY);
    // Delay for a short time before repeating the loop
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void printTask(void *pvParameters) {
  // Loop forever
  while (1) {
    // Create a JSON document
    char jsonStringBuffer[JSON_BUFFER_SIZE];
    xQueueReceive(queueHandle, &jsonStringBuffer, portMAX_DELAY);
    Serial.println(jsonStringBuffer);
    //vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void receiveTask(void *pvParameters) {
  // Loop forever
  while (1) {
    //read from serial, expecting key setT and value
    //if key is setT, set the outside temperature to the value
    if (Serial1.available()>0)
    {
      String input = Serial1.readStringUntil('\r');
      Serial1.println("raw input: " + input);
      StaticJsonDocument<20> doc;
      DeserializationError error = deserializeJson(doc, input);
      if (error) {
        Serial1.print(F("deserializeJson() failed: "));
        Serial1.println(error.c_str());
      }
      else
      {
        if(doc.containsKey("setT"))
        {
          float setT = doc["setT"];
          desiredTemperature = setT;
          Serial1.print(F("setT: "));
          Serial1.println(setT);
        }
      }
    }
    taskYIELD();
  }
}
