#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

#include <esp32-hal-timer.h>
#include "CoinDetector.h"


const char *TAG = "app";


LiquidCrystal_I2C *lcd;
IRAM_ATTR CoinDetector *detector;

void IRAM_ATTR hookStart() {
    log_d("hookStart");
//    detector->itrStart();
}

void IRAM_ATTR hookEnd() {
    log_d("hookEnd");
//    detector->itrEnd();
    log_d("itrEnd");
    log_e("%s", __func__);
    BaseType_t xHigherPriorityTaskWoken, xResult;
    xResult = xEventGroupSetBitsFromISR(detector->eventHandle, CoinDetector::BIT_END, &xHigherPriorityTaskWoken);

    log_i("res %d", xResult);
    if (detector->state != CoinDetector::State::COIN_PASS_MEASURE_SENSOR) {
        log_e("itrEnd invalid state");
        return;
    }


    timerStop(detector->coinTimer);
    detector->lastMicros = timerReadMicros(detector->coinTimer);


    detector->state = CoinDetector::State::COIN_NOT_IN_CHANNEL;
    if (xResult != pdFAIL) {
        log_i("event sent");
        portYIELD_FROM_ISR();
    } else {
        log_i("event not sent");
    }
}

void IRAM_ATTR hookTimer() {
    log_d("hookTimer");
//    detector->itrTimer();
}

void callbackFail() {
    ESP_LOGI(TAG, "fail");
}

void callbackSuccess(uint64_t micros, uint16_t adc) {
    ESP_LOGI(TAG, "success %d %d", micros, adc);
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
    CoinDetector::Config detectorCfg = {
            .gpioStart = 34,
            .gpioMeasure = 35,
            .gpioEnd = 32,
            .timerId = 2,
            .hookStart = hookStart,
            .hookEnd = hookEnd,
            .hookTimer = hookTimer,
            .callbackSuccess = callbackSuccess,
            .callbackFail = callbackFail,

    };
    detector = new CoinDetector(detectorCfg);
}

void loop() {
    vTaskDelay(100);
}
