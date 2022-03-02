//
// Created by Victor on 23.02.2022.
//
#include "CoinDetector.h"

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/portmacro.h>

#include <esp32-hal-log.h>
#include <esp32-hal-gpio.h>
#include "driver/timer.h"

#define ADC_BUFF_SIZE   8

static EventGroupHandle_t eventHandle;
static TaskHandle_t workerHandle;
static hw_timer_t *coinTimer;
static hw_timer_t *adcTimer;

static volatile StateEnum_t state = CD_COIN_NOT_IN_CHANNEL;
static cd_config_t config;

static volatile uint64_t lastMicros = 0;

static volatile uint16_t adcIdx = 0;
static volatile uint16_t maxAdc = 0;
static volatile uint16_t adcData[ADC_BUFF_SIZE] = {0};


static void IRAM_ATTR itrStart();

static void IRAM_ATTR itrEnd();

static void IRAM_ATTR itrTimerCoin();

static void IRAM_ATTR itrTimerAdc();

_Noreturn static void workerTask(void *pvParameters);

void cd_init(cd_config_t newConfig) {
    config = newConfig;

    //init GPIOs
    pinMode(config.gpioStart, INPUT);
    pinMode(config.gpioMeasure, INPUT);
    pinMode(config.gpioEnd, INPUT);


    //init adc timer
    adcTimer = timerBegin(config.timerNumAdc, 80, true);
    timerAttachInterrupt(adcTimer, itrTimerAdc, RISING);
    timerAlarmWrite(adcTimer, 500, true);
    timerAlarmEnable(adcTimer);
    timerStop(adcTimer);


    coinTimer = timerBegin(config.timerNumCoin, getApbFrequency() / 1000000, true); //setup timer to 1 micros precision
    timerStop(coinTimer);

    eventHandle = xEventGroupCreate();
    xTaskCreatePinnedToCore(workerTask, "coinWorker", 4096, NULL, 5, &workerHandle, 0);

    attachInterrupt(config.gpioStart, itrStart, FALLING);
    attachInterrupt(config.gpioEnd, itrEnd, FALLING);

//    timerAttachInterrupt(coinTimer, itrTimerCoin, RISING);


}


void itrStart() {
    log_d("itrStart");
    if (state != CD_COIN_NOT_IN_CHANNEL) {
//        log_e("itrStart invalid state");
        return;
    }

    timerRestart(coinTimer);
    timerStart(coinTimer);
    state = CD_COIN_PASS_START_SENSOR;  //TODO check measure sensor
    state = CD_COIN_PASS_MEASURE_SENSOR;

    BaseType_t xHigherPriorityTaskWoken, xResult;
    xResult = xEventGroupSetBitsFromISR(eventHandle, CD_BIT_START, &xHigherPriorityTaskWoken);
    if (xResult) {
//        log_i("event sent");
        portYIELD_FROM_ISR();
    }
}

void itrEnd() {
    log_d("itrEnd");

    if (state != CD_COIN_PASS_MEASURE_SENSOR) {
//        log_e("itrEnd invalid state");
        return;
    }

    uint64_t value;
    timer_get_counter_value(0, 0, &value);

    lastMicros = timerRead(coinTimer);
//    log_i("itr va: %lu", value);
//    log_i("itr micros: %lu", lastMicros);
//    lastMicros = timerReadMicros(coinTimer);


    state = CD_COIN_NOT_IN_CHANNEL;

    BaseType_t xHigherPriorityTaskWoken, xResult;
    xResult = xEventGroupSetBitsFromISR(eventHandle, CD_BIT_END, &xHigherPriorityTaskWoken);
    if (xResult != pdFAIL) {
//        log_i("event sent");
        portYIELD_FROM_ISR();
    } else {
//        log_i("event not sent");
    }


}


void itrTimerCoin() {
    log_d("itrTimerCoin");
    if (!(state == CD_COIN_PASS_START_SENSOR ||
          state == CD_COIN_PASS_MEASURE_SENSOR)) {
        log_e("itrTimerCoin invalid state");
    }

    BaseType_t xHigherPriorityTaskWoken, xResult;
    xResult = xEventGroupSetBitsFromISR(eventHandle, CD_BIT_FAIL, &xHigherPriorityTaskWoken);
    if (xResult) {
//        log_i("event sent");
        portYIELD_FROM_ISR();
    }
}

void itrTimerAdc() {
//    log_i("itrTimerAdc");
    adcData[adcIdx] = analogRead(config.gpioMeasure);


    adcIdx++;
    if (adcIdx >= ADC_BUFF_SIZE) {
        adcIdx = 0;
    }

    uint32_t sum = 0;
    for (int i = 0; i < ADC_BUFF_SIZE; ++i) {
        sum += adcData[i];
    }
    sum /= ADC_BUFF_SIZE;
//    log_i("val: %d max: %d sum: %d", adcData[adcIdx], maxAdc, sum);
    if (maxAdc < sum) {
        maxAdc = sum;
    }
}

_Noreturn static void workerTask(void *pvParameters) {
    log_i("workerTask init");
    while (true) {
//        vTaskDelay(1000);

//        yield();
//        vTaskDelay(1);
        EventBits_t uxBits = xEventGroupWaitBits(eventHandle,
                                                 CD_BIT_START | CD_BIT_END | CD_BIT_FAIL,
                                                 pdTRUE,
                                                 pdFALSE,
                                                 portMAX_DELAY);
//        log_i("read event");
//        log_i("max: %hu", maxAdc);
//        log_i("val: %lu", lastMicros);

        if (uxBits & CD_BIT_START) {
//            log_i("got start evt");
            maxAdc = 0;
            lastMicros = 0;
            memset((void *) adcData, 0, ADC_BUFF_SIZE);

            timerRestart(adcTimer);
            timerStart(adcTimer);

        }
        if (uxBits & CD_BIT_END) {
//            log_i("got end evt");
            timerStop(adcTimer);
            timerStop(coinTimer);
            config.callbackSuccess(lastMicros, maxAdc);
        }
        if (uxBits & CD_BIT_FAIL) {
//            log_i("got fail evt");
            config.callbackFail();
        }

    }
}
