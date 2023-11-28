
/*This code is for users information device 
It works based on ESP-NOW  protocol and display's the information in OLED display */


#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_SH1106.h>

//defining pins for OLED display 
//OLED uses SH1106 driver and SPI bus protocol
#define OLED_SDA 23 //MOSI --> Master Out Slave In 
#define OLED_SCK 18  
#define OLED_DC 16
#define OLED_RST 17
#define OLED_CS 5

// creating an instance for oled 
Adafruit_SH1106 display(OLED_SDA,OLED_SCK,OLED_DC,OLED_RST,OLED_CS);

#if(SH1106_LCDHEIGHT != 64)
 #error("correct the display");
#endif
//callback function for espnow 
void OnDataRecv(const uint8_t *mac,const uint8_t *incoming , int len)
{
  //deserializing json data
  StaticJsonDocument <520> jsondoc;
  deserializeJson(jsondoc , incoming );
  int temp = jsondoc["Temperature"];
  int hum = jsondoc["Humidity"];
  int MQ_7 = jsondoc["MQ-7"];
  String time = jsondoc["Time"];

  if(temp <35 || hum <40 || MQ_7 < 120) //displaying data from helmet
  {
    display.setTextSize(1.5);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print("Temp :");
    display.println(temp);
    display.print("\n Hum :");
    display.println(hum);
    display.print("%");
    display.print("\n Air_quality :");
    display.println(MQ_7);
    display.print("\n Time :");
    display.println(time);
    display.display();
  }
  else
  {
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print("Danger !!!");
    display.display();
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Started!!!!!!!!!");

  display.begin(SH1106_SWITCHCAPVCC); //intialing display
  display.clearDisplay();   // clearing buffer fo display
  //intitiating espnow 
  if(esp_now_init()!= ESP_OK)
  {
    Serial.println("Error intializing ESP_NOW !!!!");
  }
  else{
    Serial.println("connected.............");
  }
  esp_now_register_recv_cb(OnDataRecv);  // registering callback function 
}

void loop()
{

}