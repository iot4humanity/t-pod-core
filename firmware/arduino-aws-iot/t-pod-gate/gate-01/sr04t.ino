void LedControl(bool mRed, bool mGreen, bool mBlue){
  digitalWrite(lredpin,mRed);
  digitalWrite(lgreenpin,mGreen);
  digitalWrite(lbluepin,mBlue);
}

void PumpControl(bool mPump){
  digitalWrite(pumppin,mPump);
}
void BuzzerControl(bool mBuzz, unsigned char mBuzzLong){
  bool mBuzzBeat = false;
  tick = 0;
  while(tick <= mBuzzLong){
    mBuzzBeat = mBuzz == true ? !mBuzzBeat : true;    
    digitalWrite(buzzpin,mBuzzBeat);
    vTaskDelay(100 / portTICK_RATE_MS); 
    tick++;
  }
  digitalWrite(buzzpin,false);
}

void Mqtt_Send(char *mDataString){
    unsigned long last_millis = millis();
    Serial.print("MQTT DATA : "); Serial.println(mDataString);
    while((WISE_Mqtt.publish(TOPIC_NAME,payload) != 0 )&& millis()-last_millis < TIMEOUT_MQTT );
    
    if(WISE_Mqtt.publish(TOPIC_NAME,payload) != 0 ){
      digitalWrite(lbluepin,1);
      delay(500);
      digitalWrite(lbluepin,0);
    }
//      CountMqtt++;
//      if (CountMqtt > 10) modeOnline = false;
//    }
}


void printLocalTime()
{
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    return;
  }
//  Serial.println(&timeinfo,"%A, %B %d %Y %H:%M:%S");
}
