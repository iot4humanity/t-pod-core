void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      client.publish("/icircuit/presence/ESP32/", "hello world");
      // ... and resubscribe
      client.subscribe(TOPIC_SUBS);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

int publishData(float Data){
  if (WiFi.status() != WL_CONNECTED ) {
    modeOnline = false;
    setup_wifi();
  }
  if(!modeOnline){
    return 0;
  }
  if (!client.connected()) {
    reconnect();
  }
  printLocalTime();
//  strptime(timestamp, "%Y-%m-%d %H:%M:%S", &tm);
  epoch = mktime(&timeinfo);
  sprintf(payload,"{\"timestamp\":%ld,\"deviceId\":\"device2\",\"temperature\":%0.2f}",(long)epoch,MaxDataTemperature);
  client.publish(TOPIC_PUBLISH, payload);
  return 1;
}
