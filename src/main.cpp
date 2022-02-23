#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

#include <esp32-hal-timer.h>


const char* TAG = "app";

hw_timer_t *coinTimer;

LiquidCrystal_I2C *lcd;


void setup() {
    Serial.begin(115200);
    Wire.begin(27, 26);

    lcd = new LiquidCrystal_I2C(0x27, 16, 2);

    lcd->init();
    lcd->clear();
    lcd->backlight();


    lcd->print("asd");
    pinMode(32, INPUT);
    pinMode(34, INPUT);

    coinTimer = timerBegin(2, 800, true);
    
}

void loop() {
    uint16_t sensor = analogRead(35);
    bool in = digitalRead(34);
    bool out = digitalRead(32);

//    lcd->setCursor(0,0);
//    lcd->print(in);
//    lcd->print(" ");
//    lcd->print(sensor);
//    lcd->print(" ");
//    lcd->print(out);
    if(in){
        lcd->clear();
        timerRestart(coinTimer);
    }
    if (out){
        uint64_t val1 = timerReadMicros(coinTimer);
        uint64_t val = timerRead(coinTimer);
        Serial.println(val);
        lcd->setCursor(0,0);
        lcd->print(val);
        lcd->setCursor(0,1);
        lcd->print(val1);
    }

}