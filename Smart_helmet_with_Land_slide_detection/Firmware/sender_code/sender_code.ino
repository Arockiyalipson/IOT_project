#include <esp_now.h>
#include <WiFi.h>
#include <DHT.h>
#include <DHT_U.h>
#include <LoRa.h>
#include <SPI.h>
#include <TinyGPS++.h>

#define GPS_BAUDRATE 9600  // The default baudrate of NEO-6M is 9600

TinyGPSPlus gps;  // the TinyGPS++ object

//defining pin and dhttype for dht sensor
#define pin 4
#define DHTTYPE DHT11

//defining pins for lora
#define ss 5
#define rst 14
#define di0 2

// defining pin for sensors and lights
int mq_9 = 35;      //connect to analog pins
int mq_135 = 34;    // connect to analog pins
int emer_pin = 33;  // connecto to analog pin
int counter = 1;    //packet id
int red = 27;
int green = 26;

//earthquake state
int earth_quake = 0;
int emer = 0;

//creating time delay for dht to read
unsigned long previousMillis = 0;     // Stores last time temperature was published
const unsigned long interval = 1000;  // interval the sensor reads or publishes the value

//creating instance for dht
DHT dht(pin, DHTTYPE);

//defining variable type
typedef struct struct_message {
  float b, c, d, e, f, h, j, M, J, K, L;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  float M, e, f, h;
  M = abs(myData.e) + abs(myData.f) + abs(myData.h);
  if (M > 0.06) {
    Serial.println("xxxxx  LANDSLIDE WARNING  xxxxx");
    earth_quake = true;
    digitalWrite(green, LOW);
    digitalWrite(red, HIGH);
    delay(5000);
    digitalWrite(red, LOW);
    digitalWrite(green, HIGH);
  } else {
    earth_quake = false;
  }
}

void lorasetup() {
  LoRa.setPins(ss, rst, di0);
  //checking for lora
  while (!LoRa.begin(433E6)) {
    Serial.print(".");
    delay(500);
  }
  LoRa.setSyncWord(0XF3);
  Serial.println("LoRa initialization ok !!");
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(GPS_BAUDRATE);
  Serial.println(F("ESP32 - GPS module"));
  lorasetup();  //lora initialization function call
  dht.begin();  //dht sensor initialization

  pinMode(mq_9, INPUT);
  pinMode(mq_135, INPUT);
  pinMode(emer_pin, INPUT);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);

  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("ESP-NOW connected sucessfully!!!!!");
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  float latitude = 0;
  float longitude = 0;
  if (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
      }
    }

    if (millis() > 5000 && gps.charsProcessed() < 10) {
      Serial.println(F("No GPS data received: check wiring"));
    }
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    double temperature = dht.readTemperature();  // reading temperature value from dht sensor
    Serial.print("temperature:");
    Serial.println(temperature);
    double humidity = dht.readHumidity();  // reading humidity value from dht sensor
    Serial.print("Humidity:");
    Serial.println(humidity);

    int MQ_9 = analogRead(mq_9);      // reading value from MQ7 sensor
    int MQ_135 = analogRead(mq_135);  // reading value from MQ 135 sensor

    //emergency button
    int Emergency = 0;
    Emergency = digitalRead(emer_pin);
    if (Emergency == 1) {
      emer = true;
      Serial.println("!!!!EMERGENCY!!!!");
      digitalWrite(red, HIGH);
      digitalWrite(green, LOW);
      delay(5000);
      digitalWrite(red, LOW);
      digitalWrite(green, HIGH);
    } else {
      emer = false;
    }

    float latitude = 0;
    float longitude = 0;
    latitude = gps.location.lat();
    Serial.println("latitude");
    Serial.println(latitude);
    longitude = gps.location.lng();
    Serial.println("longitude");
    Serial.println(longitude);


    // writing all value into a JSON document or structure
    String data = "{\"Temperature\" : \"" + String(temperature) + "\",\"Humidity\" : \"" + String(humidity) + "\",\"MQ-9\" : \"" + String(MQ_9) + "\" , \"MQ-135\" : \"" + String(MQ_135) + "\" ,\"Packet_id\" : \"" + String(counter) + "\",\"earth\" : \"" + String(earth_quake) + "\",\"emergency\" : \"" + String(emer) + "\",\"longitude\" : \"" + String(longitude) + "\",\"latitude\" : \"" + String(latitude) + "\"}";
    //Sending via LoRa
    Serial.println(data);  // printing lora data
    LoRa.beginPacket();
    LoRa.print(data);  // sending value via lora
    LoRa.endPacket();
    counter++;  // counter for packet id
  }
}