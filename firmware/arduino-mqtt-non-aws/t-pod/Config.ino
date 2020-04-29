void GPIO_Init(void){
    Serial.begin(115200);
    lcd.begin();
    mlx.begin(); 
    // Define inputs and outputs
    pinMode(irsensin, INPUT);
    pinMode(lredpin, OUTPUT);
    pinMode(lbluepin, OUTPUT);
    pinMode(lgreenpin, OUTPUT);
    pinMode(pumppin, OUTPUT);
    pinMode(buzzpin, OUTPUT);
    
    digitalWrite(buzzpin,true);
    delay(2000);
    digitalWrite(buzzpin,false);
    lcd.backlight();
    lcd.setCursor(5,0);
    lcd.print("DETEKSI");
    lcd.setCursor(6,1);
    lcd.print("SUHU");
    delay(2000);
    digitalWrite(lredpin,true); delay(500); digitalWrite(lredpin,false);
    digitalWrite(lgreenpin,true); delay(500); digitalWrite(lgreenpin,false);
    digitalWrite(lbluepin,true); delay(500); digitalWrite(lbluepin,false);
    
}

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    unsigned long last_millis,timing;
    lcd.clear();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    modeOnline ? timing = 30000 : timing = 2000;
    WiFi.begin(ssid, password);
    last_millis = millis();
    while (WiFi.status() != WL_CONNECTED &&  millis() - last_millis < timing) {
      delay(300);
      Serial.print(".");
    }
    if (WiFi.status() != WL_CONNECTED ) {
          modeOnline = false;
          lcd.setCursor(0,0); lcd.print("AP not Found     ");
          lcd.setCursor(0,1); lcd.print("Offline Mode     ");
          delay(1000);
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
