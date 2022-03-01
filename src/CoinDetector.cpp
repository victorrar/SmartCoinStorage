//
// Created by Victor on 23.02.2022.
//

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/portmacro.h>

#include <esp32-hal-log.h>
#include <esp32-hal-gpio.h>
#include "CoinDetector.h"

CoinDetector::CoinDetector(const Config &config) : config(config) {
    init();
    log_i("Detector construct success");

}

void CoinDetector::init() {
    //init GPIOs
    pinMode(config.gpioStart, INPUT);
    pinMode(config.gpioMeasure, INPUT);
    pinMode(config.gpioEnd, INPUT);
    coinTimer = timerBegin(config.timerId, getApbFrequency() / 1000000, true); //setup timer to 1 micros precision



    eventHandle = xEventGroupCreate();
    xTaskCreate(workerTask, "coinWorker", 4096, this, 5, &workerHandle);

    attachInterrupt(config.gpioStart, config.hookStart, FALLING);
    attachInterrupt(config.gpioEnd, config.hookEnd, FALLING);

    timerAttachInterrupt(coinTimer, config.hookTimer, RISING);
}

void IRAM_ATTR CoinDetector::itrStart() {
    log_d("itrStart");
    if (state != State::COIN_NOT_IN_CHANNEL) {
        log_e("itrStart invalid state");
        return;
    }
    timerRestart(coinTimer);
    timerStart(coinTimer);
//    state = State::COIN_PASS_START_SENSOR;  //TODO check measure sensor
    state = State::COIN_PASS_MEASURE_SENSOR;
}

void IRAM_ATTR CoinDetector::itrEnd() {
    log_d("itrEnd");
    BaseType_t xHigherPriorityTaskWoken, xResult;
    xResult = xEventGroupSetBitsFromISR(eventHandle, BIT_END, &xHigherPriorityTaskWoken);

    log_i("res %d", xResult);
    if (state != State::COIN_PASS_MEASURE_SENSOR) {
        log_e("itrEnd invalid state");
        return;
    }


    timerStop(coinTimer);
    lastMicros = timerReadMicros(coinTimer);


    state = State::COIN_NOT_IN_CHANNEL;
    if (xResult != pdFAIL) {
        log_i("event sent");
        portYIELD_FROM_ISR();
    } else {
        log_i("event not sent");
    }


}

void IRAM_ATTR CoinDetector::itrTimer() {
    log_d("itrTimer");
    if (!(state == State::COIN_PASS_START_SENSOR ||
          state == State::COIN_PASS_MEASURE_SENSOR)) {
        log_e("itrTimer invalid state");
    }
    BaseType_t xHigherPriorityTaskWoken, xResult;

    xResult = xEventGroupSetBitsFromISR(eventHandle, BIT_FAIL, &xHigherPriorityTaskWoken);
    if (xResult != pdFAIL) {
        log_i("event sent");
//        portYIELD_FROM_ISR();
    }
}

[[noreturn]] void CoinDetector::workerTask(void *pvParameters) {
    auto *owner = (CoinDetector *) pvParameters;
    log_i("workerTask init");
    while (true) {
//        vTaskDelay(1000);

        yield();
        vTaskDelay(1);
        EventBits_t uxBits = xEventGroupWaitBits(owner->eventHandle,
                                                 BIT_START | BIT_END | BIT_FAIL,
                                                 pdTRUE,
                                                 pdFALSE,
                                                 portMAX_DELAY);
        log_i("read event");
        /*
        if (uxBits & BIT_START) {
            log_i("got start evt");
            //TODO start ADC
        }
        if (uxBits & BIT_END) {
            log_i("got end evt");
            owner->config.callbackSuccess(owner->lastMicros, owner->lastADC);
        }
        if (uxBits & BIT_FAIL) {
            log_i("got fail evt");
            owner->config.callbackFail();
        }
         */
    }
}

