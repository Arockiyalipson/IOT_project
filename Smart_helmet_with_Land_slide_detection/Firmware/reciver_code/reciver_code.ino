#include <WiFiClientSecure.h>
#include <esp_crt_bundle.h>
#include <ssl_client.h>
#include <FB_Const.h>
#include <FB_Error.h>
#include <FB_Network.h>
#include <FB_Utils.h>
#include <Firebase.h>
#include <FirebaseFS.h>
#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>


//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define di0 2
//variable to store previous time
unsigned long prevmills = 0;

// wifi credentials
#define ssid "Networkchuck"
#define pass "inpbq2304"


//credentials for connecting firebae db
#define db_url "https://mini-project-c1b66-default-rtdb.firebaseio.com/"
#define apikey "AIzaSyDyzHITWPRMkjf8jcBg9XnLZ7aDzBfPdYw"
bool signup = false;
// creating instances
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WiFiClient Client;

void lorasetup()  // lora initializaton
{
  LoRa.setPins(ss, rst, di0);
  while (!LoRa.begin(433E6)) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("LoRa initialization ok !!");
  LoRa.setSyncWord(0XF3);
}

void wifisetup()  //WiFi setup
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.println("connecting to wifi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("IP address:");
  Serial.println(WiFi.localIP());
}


void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  lorasetup();
  wifisetup();
  config.api_key = apikey;       // assign api key to config api object
  config.database_url = db_url;  //assign realtime database url  to config database object

  //signup setup
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("signup complete!!");
    signup = true;
  } else {
    Serial.println(config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.reconnectWiFi(true);
  Firebase.begin(&config, &auth);
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // read packet
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();  // reading value of LORA
      Serial.println(LoRaData);

      StaticJsonDocument<512> doc;
      deserializeJson(doc, LoRaData);

      // // deserializing JSON data JSON ---> integer value
      int temp = doc["Temperature"];
      int hum = doc["Humidity"];
      int MQ_9 = doc["MQ-9"];
      int MQ_135 = doc["MQ-135"];
      int packet_id = doc["Packet_id"];
      String time = doc["Time"];
      // float latitude = doc["latitude"];
      // float longitude = doc["longitude"];
      int alert = doc["earth"];
      int emergency = doc["emergency"];
      Serial.println(alert);
      Serial.println(emergency);

      Serial.println("Received packet");  //acknowledgemet of recieved message
      Serial.print("\n packet_ID :");
      Serial.println(packet_id);
      if (signup && Firebase.ready() && (millis() - prevmills >= 1000 || prevmills == 0)) {
        prevmills = millis();
        int temp_status = Firebase.RTDB.setInt(&fbdo, "data/temp", temp);
        int hum_status = Firebase.RTDB.setInt(&fbdo, "data/hum", hum);
        int mq7_status = Firebase.RTDB.setInt(&fbdo, "data/mq_9", MQ_9);
        int mq135_status = Firebase.RTDB.setInt(&fbdo, "data/mq_135", MQ_135);
        int id_status = Firebase.RTDB.setInt(&fbdo, "data/ID", packet_id);
        // int lat_status = Firebase.RTDB.setInt(&fbdo, "data/latitude", latitude);
        // int long_status = Firebase.RTDB.setInt(&fbdo, "data/longitude", longitude);
        int alert_status = Firebase.RTDB.setInt(&fbdo, "data/earth", alert);
        int emergency_status = Firebase.RTDB.setInt(&fbdo, "data/emergency", emergency);
        Serial.println(alert_status);
        if ((alert_status) == 200) {
          Serial.print("success");
          Serial.println(packet_id);
        } else {
          Serial.println(fbdo.errorReason());
        }
      }


      Serial.println("-----------------------------------------------------------------------------------------");
    }
  }
}