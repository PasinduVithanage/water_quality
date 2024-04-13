#include <WiFi.h>
#include <ThingSpeak.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
const char* ssid = "V2027";
const char* password = "pasindu123";

// ThingSpeak credentials
unsigned long myChannelNumber = 2447117;
const char* myWriteAPIKey = "1R6LDPQ483QUGFMI";

// Sensor Pins
const int pH_PIN = 34;
const int sensorPin = 35; // Turbidity sensor pin

// Temperature sensor setup
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// LCD display
LiquidCrystal_I2C lcd(0x27, 2, 16);

// Calibration and sensor reading variables
float calibration_value = 20.64; // Adjust as needed

// Initialize WiFi client
WiFiClient client;

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

float readpH() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(pH_PIN);
    delay(10);
  }
  float avg = sum / 10.0;
  float voltage = avg * 3.3 / 4095.0;
  float phValue = -5.70 * voltage + calibration_value;
  return phValue;
}

int readTurbidity() {
  int sensorValue = analogRead(sensorPin);
  return map(sensorValue, 0, 750, 100, 0);
}

float readTemperature() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}

void sendDataToThingSpeak(float phValue, float temperature, int turbidity) {
  ThingSpeak.setField(1, phValue);
  ThingSpeak.setField(2, temperature);
  ThingSpeak.setField(3, turbidity);
  if (ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey) == 200) {
    Serial.println("Data sent to ThingSpeak");
  } else {
    Serial.println("Error sending data to ThingSpeak");
  }
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  sensors.begin();
  connectToWiFi();
  ThingSpeak.begin(client);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  float phValue = readpH();
  float temperatureC = readTemperature();
  int turbidityValue = readTurbidity();

  Serial.print("pH Value: ");
  Serial.println(phValue);
  Serial.print("Temperature Value: ");
  Serial.println(temperatureC);
  Serial.print("Turbidity Value: ");
  Serial.println(turbidityValue);

  sendDataToThingSpeak(phValue, temperatureC, turbidityValue);

  delay(15000); // Delay between updates to comply with ThingSpeak's update rate limits
}
