#include <WiFi.h>
#include "DHTesp.h"
#include "ThingSpeak.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Sensor pin definitions
const int DHT_PIN = 23;    // DHT22 data pin
const int MQ135_PIN = 34;  // MQ135 analog output (A0)

// OLED I2C pins
#define OLED_SDA 21  // SDA connected to GPIO 21
#define OLED_SCL 22  // SCL connected to GPIO 22

// OLED display setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C  // OLED I2C address

// Set a threshold for gas detection (adjust based on testing)
const int GAS_THRESHOLD = 300; // Example value, calibrate as needed

// WiFi credentials
const char* WIFI_SSID_1 = "Mobile WiFi_Vi_4507";
const char* WIFI_PASSWORD_1 = "12345678";

const char* WIFI_SSID_2 = "Gaurav's S24"; // Replace with second WiFi name
const char* WIFI_PASSWORD_2 = "aayushisgay"; // Replace with second WiFi password

// ThingSpeak credentials
const int myChannelNumber = 2828891;
const char* myApiKey = "R5QY1II4636ZYP2A";
const char* server = "api.thingspeak.com";

// Sensor objects
DHTesp dhtSensor;
WiFiClient client;
TwoWire I2C_OLED = TwoWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_OLED, OLED_RESET);

// Function to connect to WiFi (tries both networks)
void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  
  WiFi.begin(WIFI_SSID_1, WIFI_PASSWORD_1);
  for (int i = 0; i < 10; i++) { // Try for 10 seconds
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to " + String(WIFI_SSID_1));
      return;
    }
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("\nFailed to connect to " + String(WIFI_SSID_1) + ". Trying " + String(WIFI_SSID_2));
  
  WiFi.begin(WIFI_SSID_2, WIFI_PASSWORD_2);
  for (int i = 0; i < 10; i++) { // Try for 10 seconds
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to " + String(WIFI_SSID_2));
      return;
    }
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nFailed to connect to any WiFi network.");
}

void setup() {
  Serial.begin(115200);
  
  // Initialize DHT22 sensor
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  
  // Connect to WiFi
  connectWiFi();
  Serial.println("Local IP: " + String(WiFi.localIP()));

  // Wait for MQ135 to warm up
  pinMode(MQ135_PIN, INPUT);
  delay(20000); // 20 seconds warm-up time
  
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);

  // Initialize OLED
  I2C_OLED.begin(OLED_SDA, OLED_SCL, 400000);  // Correct I2C Pins
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Initializing...");
  display.display();
  delay(2000);
}

void loop() {
  delay(2000); // Allow sensors to stabilize

  // Read DHT22 sensor data
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  if (isnan(data.temperature) || isnan(data.humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Read raw analog value from MQ135
  int rawValue = analogRead(MQ135_PIN);
  
  // Convert raw value to approximate CO level in PPM
  float ppm = (rawValue / 4095.0) * 1000; // Example conversion (adjust with calibration)

  // Detect gas presence based on threshold
  int gasDetected = (rawValue > GAS_THRESHOLD) ? 1 : 0;

  // Debug output
  Serial.println("Temp: " + String(data.temperature, 2) + "°C");
  Serial.println("Humidity: " + String(data.humidity, 1) + "%");
  Serial.println("Raw MQ135 Value: " + String(rawValue));
  Serial.println("CO Level (Approximate): " + String(ppm) + " ppm");
  Serial.println("Gas Presence: " + String(gasDetected));

  // Set ThingSpeak fields
  ThingSpeak.setField(1, data.temperature);
  ThingSpeak.setField(2, data.humidity);
  ThingSpeak.setField(3, ppm);  // CO level in ppm
  ThingSpeak.setField(4, gasDetected); // Gas detected (0 = No, 1 = Yes)

  // Write fields to ThingSpeak
  int x = ThingSpeak.writeFields(myChannelNumber, myApiKey);
  if (x == 200) {
    Serial.println("Data pushed successfully");
  } else {
    Serial.println("Push error: " + String(x));
  }

  // Update OLED display
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("ESP32 Sensor Data");
  display.setCursor(0, 20);
  display.println("Temp: " + String(data.temperature, 2) + " C");
  display.setCursor(0, 30);
  display.println("Humidity: " + String(data.humidity, 1) + " %");
  display.setCursor(0, 40);
  display.println("CO Level: " + String(ppm) + " ppm");
  display.setCursor(0, 50);
  display.print("Gas: ");
  display.println(gasDetected ? "YES" : "NO");
  display.display();

  Serial.println("---");
  delay(10000); // Wait 10 seconds before next reading
}