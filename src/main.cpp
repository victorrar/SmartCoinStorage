#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

#include <esp32-hal-timer.h>

#include "CoinDetector.h"
#include "CoinSorter.h"
#include "AlgoSorter.h"



const char *TAG = "app";


static LiquidCrystal_I2C *lcd;
static AlgoSorter *sorter;

static void callbackSuccess(uint64_t micros, uint16_t adc);

static void callbackFail();

void callbackFail() {
    ESP_LOGI(TAG, "fail");
}

void callbackSuccess(uint64_t micros, uint16_t adc) {
    log_i("success");
    log_i("micros %lu", micros);
    log_i("adc %hu", adc);

    CoinSorter::CoinDescriptor *descriptor = sorter->Predict(micros, adc);
//
    ESP_LOGI(TAG, "Predict: %s %d", descriptor->displayName, descriptor->value);
}



void setup() {
    Serial.begin(115200);

    //lcd init
    Wire.begin(27, 26);

    lcd = new LiquidCrystal_I2C(0x27, 16, 2);

    lcd->init();
    lcd->clear();
    lcd->backlight();

    lcd->print("asd");

    //detector init
    cd_config_t detectorCfg = {
            .gpioStart = 34,
            .gpioMeasure = 35,
            .gpioEnd = 32,
            .timerNumCoin = 0,
            .timerNumAdc = 1,
            .callbackSuccess = callbackSuccess,
            .callbackFail = callbackFail,
    };

//    detectorCfg.callbackSuccess(3,4);

    sorter = new AlgoSorter();
    sorter->AddCoin({.displayName = "2.00", .value = 200, .adcValue =  2819, .time = 150260});
//    sorter->AddCoin({.displayName = "5.00", .value = 500, .adcValue =  3238, .time = 159431});
    sorter->AddCoin({.displayName = "10.00", .value = 1000, .adcValue =  4038, .time = 171285});

    sorter->Init();

    cd_init(detectorCfg);

}

void loop() {
    vTaskDelay(100);
}
