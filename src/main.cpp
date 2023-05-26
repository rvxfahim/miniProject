#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <ArduinoJson.h>
// #include "avr8-stub.h"
#include <queue.h>
#include <math.h>
// Define the pins for the cooling power input
const int fanSpeedpin = A0;
#define compressorA 51
#define compressorB 53
// Define the constants for the thermal model
const float wallThermalResistance = 0.005; // in K/W
const float roomVolume = 20;               // in m^3
const float roomHeatCapacity = 1000;       // in J/K
const float roomInitialTemperature = 30;   // in degC
float outsideTemperature = 30;       // in degC
bool comp[2] = {false, false};
bool initState = true;
// Define the variables for the thermal model
float insideTemperature = roomInitialTemperature;
float coolingPower = 0;
float desiredTemperature = 23;
unsigned int switchTime = 15000;
// Define the task handles
TaskHandle_t simulationTaskHandle;
TaskHandle_t printTaskHandle;
TaskHandle_t receiveTaskHandle;
TaskHandle_t compressorTaskHandle;
TaskHandle_t loadBalanceTaskHandle;
TaskHandle_t fanspeedTaskHandle;
// Define the queue handle
QueueHandle_t queueHandle;
//
StaticJsonDocument<32> doc;
StaticJsonDocument<32> doc2;
StaticJsonDocument<20> recdoc;
// Define the size of the JSON buffer
const size_t JSON_BUFFER_SIZE = JSON_OBJECT_SIZE(4) + 40;
// Define the task functions
void simulationTask(void *pvParameters);
void printTask(void *pvParameters);
void receiveTask(void *pvParameters);
void compressorTask(void *pvParameters);
void loadBalanceTask(void *pvParameters);
void fanspeedTask(void *pvParameters);
void sendAck()
{
  char jsonStringBuffer[JSON_BUFFER_SIZE];
  StaticJsonDocument<10> doc;
  // Add key-value pairs to the document
  doc["ack"] = 1;
  size_t jsonStringSize = measureJson(doc);
  serializeJson(doc, jsonStringBuffer, jsonStringSize + 1);
  // Send the JSON string to the output task
  xQueueSend(queueHandle, &jsonStringBuffer, portMAX_DELAY);
}

void setup()
{
  // Initialize the serial communication
  // debug_init();
  pinMode(fanSpeedpin, INPUT);
  pinMode(9, OUTPUT);
  Serial.begin(9600);
  Serial.setTimeout(1000);
  pinMode(compressorA, OUTPUT);
  pinMode(compressorB, OUTPUT);
  queueHandle = xQueueCreate(10, JSON_BUFFER_SIZE);

  // Create the simulation task
  xTaskCreate(simulationTask, "Simulation Task", 1024, NULL, 2, &simulationTaskHandle);

  // Create the print task
  xTaskCreate(printTask, "Print Task", 512, NULL, 2, &printTaskHandle);

  // Create receive task
  xTaskCreate(receiveTask, "Receive Task", 512, NULL, 2, &receiveTaskHandle);
  // create compressor task
  xTaskCreate(compressorTask, "Compressor Task", 256, NULL, 2, &compressorTaskHandle);
  vTaskSuspend(compressorTaskHandle);
  // create load balance task
  xTaskCreate(loadBalanceTask, "Load Balance Task", 64, NULL, 2, &loadBalanceTaskHandle);
  vTaskSuspend(loadBalanceTaskHandle);
  // create fan speed task
  xTaskCreate(fanspeedTask, "Fan Speed Task", 64, NULL, 2, &fanspeedTaskHandle);
  // Start the scheduler
  vTaskStartScheduler();
}

void loop()
{
  // This function should never be called
}

void simulationTask(void *pvParameters)
{
  // Allocate a buffer for the JSON string
  char jsonStringBuffer[JSON_BUFFER_SIZE];
  // Loop forever
  while (1)
  {
    // Calculate the heat transfer between the inside and outside
    float heatTransfer = (outsideTemperature - insideTemperature) / wallThermalResistance;

    // If cooling power is present, subtract it from the heat transfer
    if (coolingPower > 0)
    {
      heatTransfer -= coolingPower;
    }

    // Calculate the change in temperature due to heat transfer
    float temperatureChange = heatTransfer / (roomVolume * roomHeatCapacity);

    // Calculate the new inside temperature
    insideTemperature += temperatureChange;
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
    vTaskDelay( 300 / portTICK_PERIOD_MS );
  }
}

void printTask(void *pvParameters)
{
  // Loop forever
  while (1)
  {
    // Create a JSON document
    char jsonStringBuffer[JSON_BUFFER_SIZE];
    xQueueReceive(queueHandle, &jsonStringBuffer, portMAX_DELAY);
    Serial.println(jsonStringBuffer);
    // vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void receiveTask(void *pvParameters)
{
  // Loop forever
  while (1)
  {
    // read from serial, expecting key setT and value
    // if key is setT, set the outside temperature to the value
    if (Serial.available() > 0)
    {
      String input = Serial.readStringUntil('\r');
      // Serial.println("raw input: " + input);
      
      DeserializationError error = deserializeJson(recdoc, input);
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
      }
      else
      {
        if (recdoc.containsKey("setT"))
        {
          float setT = recdoc["setT"];
          desiredTemperature = setT;
          sendAck();
        }
        else if (recdoc.containsKey("start"))
        {
          // start compressor task
          sendAck();
          int start = recdoc["start"];
          if (start)
          {
            vTaskResume(compressorTaskHandle);

          }
          else
          {
            Serial.println(F("Stopped"));
            vTaskSuspend(compressorTaskHandle);
            vTaskSuspend(loadBalanceTaskHandle);
            coolingPower = 0;
            doc2["comA"] = 0;
            doc2["comB"] = 0;
            digitalWrite(compressorA, LOW);
            digitalWrite(compressorB, LOW);
            size_t jsonStringSize = measureJson(doc2);
            char jsonStringBuffer[JSON_BUFFER_SIZE];
            serializeJson(doc2, jsonStringBuffer, jsonStringSize + 1);
            // Send the JSON string to the output task
            xQueueSend(queueHandle, &jsonStringBuffer, portMAX_DELAY);
          }
        }
        else if (recdoc.containsKey("outT"))
        {
          sendAck();
          outsideTemperature = recdoc["outT"];
        }
        else if (recdoc.containsKey("switchT"))
        {
          sendAck();
          switchTime = recdoc["switchT"];
        }
        
      }
    }
     taskYIELD();
  }
}

void compressorTask(void *pvParameters)
{
  bool initStatusupdate = false;
  bool initStatusupdate2 = false;
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  while (1)
  {
    if (initState)
    {
      if (desiredTemperature < insideTemperature)
      {
        coolingPower = 5000;
        comp[0] = 1;
        comp[1] = 1;
        if (!initStatusupdate)
        {
          
          // Add key-value pairs to the document
          doc2["comA"] = 1;
          doc2["comB"] = 1;
          digitalWrite(compressorA, HIGH);
          digitalWrite(compressorB, HIGH);
          size_t jsonStringSize = measureJson(doc2);
          char jsonStringBuffer[JSON_BUFFER_SIZE];
          serializeJson(doc2, jsonStringBuffer, jsonStringSize + 1);
          // Send the JSON string to the output task
          xQueueSend(queueHandle, &jsonStringBuffer, portMAX_DELAY);
        }
        initStatusupdate = true;
      }
      else
      {
        coolingPower = 0;
        initState = false;
        comp[0] = 1; // setting compressor to 1 so to select the compressor after init and first phase complete
        comp[1] = 0;
        if (!initStatusupdate2)
        {
          
          // Add key-value pairs to the document
          doc2["comA"] = 0;
          doc2["comB"] = 0;
          digitalWrite(compressorA, LOW);
          digitalWrite(compressorB, LOW);
          size_t jsonStringSize = measureJson(doc2);
          char jsonStringBuffer[JSON_BUFFER_SIZE];
          serializeJson(doc2, jsonStringBuffer, jsonStringSize + 1);
          // Send the JSON string to the output task
          xQueueSend(queueHandle, &jsonStringBuffer, portMAX_DELAY);
        }
        initStatusupdate2 = true;
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(1500) );
        // start load balancer task
        vTaskResume(loadBalanceTaskHandle);
      }
      vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(500) );
      initStatusupdate = false;
      initStatusupdate2 = false;
    }
    else // if not init state
    { vTaskResume(loadBalanceTaskHandle);
      if (desiredTemperature < insideTemperature)
      {
      
          initStatusupdate2 = false;
          coolingPower = 2500;
          
          // Add key-value pairs to the document
          doc2["comA"] = comp[0];
          doc2["comB"] = comp[1];
          digitalWrite(compressorA, comp[0]);
          digitalWrite(compressorB, comp[1]);
          size_t jsonStringSize = measureJson(doc2);
          char jsonStringBuffer[JSON_BUFFER_SIZE];
          serializeJson(doc2, jsonStringBuffer, jsonStringSize + 1);
          // Send the JSON string to the output task
          xQueueSend(queueHandle, &jsonStringBuffer, portMAX_DELAY);
    
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(500) );
      }
      else
      {
        
          initStatusupdate = false;
          coolingPower = 0;
          
          // Add key-value pairs to the document
          doc2["comA"] = 0;
          doc2["comB"] = 0;
          digitalWrite(compressorA, LOW);
          digitalWrite(compressorB, LOW);
          size_t jsonStringSize = measureJson(doc2);
          char jsonStringBuffer[JSON_BUFFER_SIZE];
          serializeJson(doc2, jsonStringBuffer, jsonStringSize + 1);
          // Send the JSON string to the output task
          xQueueSend(queueHandle, &jsonStringBuffer, portMAX_DELAY);
        
        

        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(2000) ); // wait for 2 seconds before switching on compressor
      }
    }
  }
}
// load balancer task
void loadBalanceTask(void *pvParameters)
{ 
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  // Loop forever
  while (1)
  {
    vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(switchTime) );
    comp[0] = !comp[0];
    comp[1] = !comp[1];
  }
}
// fan speed task
void fanspeedTask(void *pvParameters)
{
  // TickType_t xLastWakeTime;
  // xLastWakeTime = xTaskGetTickCount();
  while (true)
  {
    unsigned int fanSpeed = analogRead(fanSpeedpin);
    fanSpeed = map(fanSpeed, 0, 1023, 0, 255);
    analogWrite(4, fanSpeed);
    // vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(100) );
    taskYIELD();
  }
}