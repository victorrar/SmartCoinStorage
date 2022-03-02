//
// Created by Victor on 23.02.2022.
//

#ifndef SMARTCOINSTORAGE_COINDETECTOR_H
#define SMARTCOINSTORAGE_COINDETECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp32-hal-timer.h>

typedef void (*cd_CallbackSuccessFunc)(uint64_t micros, uint16_t adc);

typedef void (*cd_CallbackFailFunc)(void);

typedef struct {
    int gpioStart;
    int gpioMeasure;
    int gpioEnd;
    uint8_t timerNumCoin;
    uint8_t timerNumAdc;
    cd_CallbackSuccessFunc callbackSuccess;
    cd_CallbackFailFunc callbackFail;

} cd_config_t;


typedef enum {
    CD_COIN_NOT_IN_CHANNEL,
    CD_COIN_PASS_START_SENSOR,
    CD_COIN_PASS_MEASURE_SENSOR,
    CD_COIN_PASS_END_SENSOR,
} StateEnum_t;

//const
const EventBits_t CD_BIT_START = (1 << 0);
const EventBits_t CD_BIT_END = (1 << 1);
const EventBits_t CD_BIT_FAIL = (1 << 2);


//public function prototypes
void cd_init(cd_config_t newConfig);

#ifdef __cplusplus
}
#endif

#endif //SMARTCOINSTORAGE_COINDETECTOR_H
