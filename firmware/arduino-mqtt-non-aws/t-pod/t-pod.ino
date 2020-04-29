#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MLX90614.h>
#include "time.h"

#define irsensin    14  //IR Sensor Proxymity
#define lredpin     25  //Led Red
#define lbluepin    32  //Led Blue
#define lgreenpin   33  //Led Green
#define pumppin     15  //Actuator Water Pump
#define buzzpin     26  //Buzzer
#define HUMAN_DETECT    0
#define TEMP_MEASURE    1
#define COVID_DETECT    2
#define SPRAY_PROCESS   3

#define OFFSET    2.3

// Update these with values suitable for your network.
const char* ssid = "emergency";
const char* password = "bismillah";
const char* mqtt_server = "mqtt.lockerwise.com";

#define mqtt_port 1883
#define MQTT_USER "lokaloo-01"
#define MQTT_PASSWORD "covid19WFH"
#define TOPIC_PUBLISH "/data/publish/esp32"
#define TOPIC_SUBS "/data/subs/esp32"

WiFiClient wifiClient;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

struct tm tm;
time_t epoch;

unsigned char Begin = false;

int status = WL_IDLE_STATUS;
int tick=0,msgCount=0,msgReceived = 0;
char payload[512];
char rcvdPayload[512];

float DataTemperature;
float MaxDataTemperature;
bool DataDistance = true;
bool DataWaterLevel = true;
bool Buzzer = false;
bool mStateTemp;

bool modeOnline  = true;

uint64_t CountUsage = 0;

struct tm timeinfo;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 21600;  //+7.00
const int   daylightOffset_sec = 3600;
char timeStringBuff[50]; //50 chars should be enough

unsigned long tik;

PubSubClient client(wifiClient);

void callback(char* topic, byte *payload, unsigned int length) {
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:");  
    Serial.write(payload, length);
    Serial.println();
}


void setup() {
  GPIO_Init();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
  tik = millis();
  Begin=HUMAN_DETECT;
}

void loop() {
   client.loop();
   switch(Begin){
    case HUMAN_DETECT :
      lcd.setCursor(0,0); lcd.print("DEKATKAN KEPALA ");
      lcd.setCursor(0,1); lcd.print("ANDA DISINI     ");
      DataDistance = digitalRead(irsensin);
      LedControl(false,false,DataDistance == 0 ? true : false);
      if(DataDistance == 0)BuzzerControl(false,5);
      Begin = DataDistance == 0 ? TEMP_MEASURE : HUMAN_DETECT;
      break;
      
    case TEMP_MEASURE :
      delay(100);
      CountUsage++;
      tick = MaxDataTemperature = 0;
      lcd.clear();
      while(tick <= 10){
        DataTemperature = LPF_DataSuhu(mlx.readObjectTempC()+ OFFSET);
        MaxDataTemperature = DataTemperature >= MaxDataTemperature ? DataTemperature : MaxDataTemperature;
        lcd.setCursor(0,0); lcd.print("SUHU = "); lcd.print(MaxDataTemperature); lcd.print(" C");
        lcd.setCursor(0,1); lcd.print("                ");
        vTaskDelay(100 / portTICK_RATE_MS); 
        tick++;
      }
 
      mStateTemp = MaxDataTemperature >= 38 ? true : false;
      LedControl(mStateTemp,!mStateTemp,false);
      
      if(mStateTemp == true)BuzzerControl(true,8);
      else BuzzerControl(false,8);
      delay(500);
      publishData(MaxDataTemperature);
      Begin = mStateTemp == true ? COVID_DETECT : SPRAY_PROCESS;
      break;

    case COVID_DETECT :
      lcd.setCursor(0,0); lcd.print("SUHU BADAN ANDA ");
      lcd.setCursor(0,1); lcd.print(MaxDataTemperature); lcd.print("        ");
      delay(2000);
      lcd.setCursor(0,0); lcd.print("SUHU BADAN ANDA ");
      lcd.setCursor(0,1); lcd.print("     TINGGI     ");
      delay(3000);
      lcd.setCursor(0,0); lcd.print("    WARNING!    ");
      lcd.setCursor(0,1); lcd.print("               ");
      delay(1000);
      
      Begin = HUMAN_DETECT;
      break;

      
    case SPRAY_PROCESS :
      LedControl(false,false,false);
      tick = 0;
      lcd.setCursor(0,0); lcd.setCursor(0,0); lcd.print("SUHU = "); lcd.print(MaxDataTemperature); lcd.print(" C");
      lcd.setCursor(0,1); lcd.print("SILAHKAN MASUK");
      delay(2000);
      
      while(tick <= 1){
        PumpControl(true);
        lcd.setCursor(0,0); lcd.print("  AWAS PROSES   ");
        lcd.setCursor(0,1); lcd.print("  STERILISASI   ");
        vTaskDelay(1000 / portTICK_RATE_MS); 
        tick++;
      }
      
      PumpControl(false);
      lcd.setCursor(0,0); lcd.print("    PROSES      ");
      lcd.setCursor(0,1); lcd.print("    SELESAI     ");
      BuzzerControl(true,4);
      delay(1000);
      lcd.setCursor(0,0); lcd.print("    SILAHKAN    ");
      lcd.setCursor(0,1); lcd.print("   BERIKUTNYA   ");
      delay(2000);
      Begin = HUMAN_DETECT;
      break;
  }
 }
