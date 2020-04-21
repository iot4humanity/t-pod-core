#include <AWS_IOT.h>
#include <WiFi.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MLX90614.h>
#include "time.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
AWS_IOT WISE_Mqtt;

#define irsensin    14  //IR Sensor Proxymity
#define lredpin     25  //Led Red
#define lbluepin    32  //Led Blue
#define lgreenpin   33  //Led Green
#define pumppin     15  //Actuator Water Pump
#define buzzpin     26  //Buzzer

#define WIFI_CONNECT    0
#define AWS_CONNECT     1
#define MQTT_CONNECT    2
#define DONE_CONNECT    3

#define TIMEOUT_MQTT    2000  //millisecond

#define HUMAN_DETECT    0
#define TEMP_MEASURE    1
#define COVID_DETECT    2
#define SPRAY_PROCESS   3

#define OFFSET    2.3

char WIFI_SSID[]="**SSID**";
char WIFI_PASSWORD[]="**PASSWORD**";
char HOST_ADDRESS[]="**HOST**";
char CLIENT_ID[]= "gate-01";
char TOPIC_NAME[]= "gate-disinfectant-01/data-01";

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

void setup() {
  unsigned long last_millis;
  GPIO_Init();
  while(Begin != DONE_CONNECT){
    switch(Begin){
      case WIFI_CONNECT :
        lcd.clear();
        last_millis = millis();
        while (status != WL_CONNECTED &&  millis() - last_millis < 30000 ){   
            lcd.setCursor(0,0); lcd.print("Search WiFi     ");
            lcd.setCursor(0,1); lcd.print(WIFI_SSID);
            Serial.print("Attempting to connect to SSID: "); Serial.println(WIFI_SSID);
            status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            delay(5000);
        }
        if (status != WL_CONNECTED ) {
          modeOnline = false;
          Begin = DONE_CONNECT;
          lcd.setCursor(0,0); lcd.print("AP not Found     ");
          lcd.setCursor(0,1); lcd.print("Offline Mode     ");
          delay(1000);
          break;
        }
        
        lcd.setCursor(0,0); lcd.print("Connected to    ");
        lcd.setCursor(0,1); lcd.print(WIFI_SSID);
        Serial.print("Connected to "); Serial.println(WIFI_SSID);
        delay(1000);
        //init and get the time
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        printLocalTime();
       
        Begin = AWS_CONNECT;
        break;
        
      case AWS_CONNECT :
        last_millis = millis();
        while(WISE_Mqtt.connect(HOST_ADDRESS,CLIENT_ID) != false && millis() - last_millis < 20000 ){
            lcd.setCursor(0,0); lcd.print("Connecting to   ");
            lcd.setCursor(0,1); lcd.print("AWS Cloud       ");
            delay(1000);
        }

        if (WISE_Mqtt.connect(HOST_ADDRESS,CLIENT_ID) != false ) {
          modeOnline = false;
          Begin = DONE_CONNECT;
          lcd.setCursor(0,0); lcd.print("cant connect AWS ");
          lcd.setCursor(0,1); lcd.print("Offline Mode     ");
          delay(1000);
          break;
        }
        
        lcd.setCursor(0,0); lcd.print("Connected to    ");
        lcd.setCursor(0,1); lcd.print("AWS Cloud       ");
        Serial.println("Connected to AWS");
        delay(1000);
        Begin = MQTT_CONNECT;
        break;

      case MQTT_CONNECT :
        last_millis = millis();
        while(WISE_Mqtt.subscribe(TOPIC_NAME,mySubCallBackHandler) != false && millis() - last_millis < 20000 )
        {
            lcd.setCursor(0,0); lcd.print("Connecting to   ");
            lcd.setCursor(0,1); lcd.print("MQTT Cloud      ");
            delay(1000);
        }

        if (WISE_Mqtt.subscribe(TOPIC_NAME,mySubCallBackHandler) != false ) {
          modeOnline = false;
          Begin = HUMAN_DETECT;
          lcd.setCursor(0,0); lcd.print("cant Subs MQTT  ");
          lcd.setCursor(0,1); lcd.print("Offline Mode    ");
          delay(1000);
          break;
        }
        
        lcd.setCursor(0,0); lcd.print("Connected to    ");
        lcd.setCursor(0,1); lcd.print("MQTT Cloud      ");
        Serial.println("Connected to MQTT");
        delay(1000);
        Begin = DONE_CONNECT;
        break;
    }
  }

  Begin = HUMAN_DETECT;
  lcd.clear();
  delay(2000);
}

void loop() {
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
      if(modeOnline){
        printLocalTime();
//      strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
//      strftime(payload, sizeof(payload), "%A, %B %d %Y %H:%M:%S", &timeinfo);
        sprintf(payload,"\"%d-%d-%d %d:%d\",\"gate-02\",%0.2f",timeinfo.tm_year+1900,timeinfo.tm_mon+1,timeinfo.tm_mday ,timeinfo.tm_hour,timeinfo.tm_min,MaxDataTemperature);
        Mqtt_Send(payload);  
      }
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

float LPF_DataSuhu(float mDataSuhu){
  static float mDataLPFSuhu;
  mDataLPFSuhu = (float)((0.7 * mDataSuhu) + (0.3 * mDataLPFSuhu));
  return mDataLPFSuhu;
}

void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad)
{
    strncpy(rcvdPayload,payLoad,payloadLen);
    rcvdPayload[payloadLen] = 0;
    msgReceived = 1;
}
