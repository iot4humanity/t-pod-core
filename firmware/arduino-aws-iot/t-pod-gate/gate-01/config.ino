void GPIO_Init(void){
    Serial.begin(115200);
    lcd.begin();
    mlx.begin(); 
    // Define inputs and outputs
    pinMode(irsensin, INPUT);
    pinMode(waterpin, INPUT);
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

